from copy import deepcopy
from functools import partial
from xml.etree import ElementTree as ET

import pytest

from qgis_headless import CRS, LT_VECTOR, Layer, MapRequest, Style
from qgis_headless.util import to_pil

from .raster_styles import raster_style_xml

SIZES = (256, 362, 512, 724, 1024)


def _style_request(layer, style, crs):
    request = MapRequest()
    request.set_dpi(96)
    request.set_crs(crs)
    request.add_layer(layer, style)
    return request


def _benchmark_style(benchmark, operation, layer, style, crs, extent):
    request = partial(_style_request, layer, style, crs)
    if operation == "used_attributes":
        benchmark(style.used_attributes)
        return
    if operation == "add_layer":
        benchmark(request)
        return
    if operation == "render":
        prepared_request = request()
        benchmark(lambda: prepared_request.render_image(extent, (512, 512)))
        return
    assert operation == "tile"
    benchmark(lambda: request().render_image(extent, (512, 512)))


@pytest.mark.benchmark(group="empty")
@pytest.mark.parametrize("size", SIZES)
def test_empty(size, benchmark):
    mreq = MapRequest()
    mreq.set_dpi(96)
    mreq.set_crs(CRS.from_epsg(3857))

    def _render_image():
        to_pil(mreq.render_image((-1, -1, 1, 1), (size, size)))

    benchmark(_render_image)


@pytest.mark.parametrize("rule_count", [1, 100, 1000])
@pytest.mark.parametrize("operation", ["add_layer", "render", "tile", "used_attributes"])
def test_style_rules(rule_count, operation, benchmark):
    benchmark.group = f"style-{operation}"
    crs = CRS.from_epsg(3857)
    layer = Layer.from_data(Layer.GT_LINESTRING, crs, (), ())
    default = Style.from_defaults(
        layer_type=LT_VECTOR,
        layer_geometry_type=Layer.GT_LINESTRING,
        color=(255, 0, 0, 255),
    )
    root = ET.fromstring(default.to_string())
    renderer = root.find("renderer-v2")
    symbol = deepcopy(renderer.find("symbols/symbol"))
    renderer.clear()
    renderer.set("type", "RuleRenderer")
    rules = ET.SubElement(renderer, "rules")
    symbols = ET.SubElement(renderer, "symbols")
    for index in range(rule_count):
        ET.SubElement(rules, "rule", {"filter": f'"value" = {index}', "symbol": str(index)})
        item = deepcopy(symbol)
        item.set("name", str(index))
        symbols.append(item)
    style = Style.from_string(ET.tostring(root, encoding="unicode"))
    # NGW reads the attributes before supplying features to the map request.
    assert style.used_attributes() == {"value"}

    _benchmark_style(benchmark, operation, layer, style, crs, (-1, -1, 1, 1))


@pytest.mark.parametrize("color_count", [1, 256, 4096])
@pytest.mark.parametrize("operation", ["add_layer", "tile"])
@pytest.mark.parametrize("source", ["rounds.tif", "cog-rgb-byte.tif"])
def test_style_palette(color_count, operation, source, benchmark, shared_datadir):
    benchmark.group = f"raster-style-{source}-{operation}"
    layer = Layer.from_gdal(shared_datadir / "raster" / source)
    root = ET.fromstring((shared_datadir / "raster/rounds.qml").read_text())
    renderer = root.find("pipe/rasterrenderer")
    renderer.clear()
    renderer.attrib.update(type="paletted", band="1", opacity="1", alphaBand="-1")
    palette = ET.SubElement(renderer, "colorPalette")
    for index in range(color_count):
        ET.SubElement(
            palette,
            "paletteEntry",
            {"value": str(index), "color": "#ff0000", "alpha": "255", "label": str(index)},
        )
    style = Style.from_string(ET.tostring(root, encoding="unicode"))
    crs = CRS.from_epsg(3857)

    _benchmark_style(benchmark, operation, layer, style, crs, (251440, 5977974, 1978853, 7505647))


@pytest.mark.parametrize(
    "renderer_type", ["multibandcolor", "singlebandgray", "singlebandpseudocolor"]
)
@pytest.mark.parametrize("operation", ["add_layer", "tile"])
def test_cog_style(renderer_type, operation, benchmark, shared_datadir):
    benchmark.group = f"cog-style-{operation}"
    root = raster_style_xml(shared_datadir, renderer_type)
    style = Style.from_string(ET.tostring(root, encoding="unicode"))
    filename = "cog-rgb-byte.tif" if renderer_type == "multibandcolor" else "cog-gray-float32.tif"
    layer = Layer.from_gdal(shared_datadir / "raster" / filename)
    crs = CRS.from_epsg(3857)

    _benchmark_style(benchmark, operation, layer, style, crs, (251440, 5977974, 1978853, 7505647))


@pytest.mark.benchmark(group="contour")
@pytest.mark.parametrize("size", SIZES)
def test_contour(size, benchmark, shared_datadir):
    data = (shared_datadir / "contour/data.geojson").read_text()
    style = (shared_datadir / "contour/simple.qml").read_text()
    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    mreq = MapRequest()
    mreq.set_dpi(96)
    mreq.set_crs(CRS.from_epsg(3857))
    mreq.add_layer(Layer.from_ogr(data), Style.from_string(style))

    def _render_image():
        to_pil(mreq.render_image(extent, (size, size)))

    benchmark(_render_image)


@pytest.mark.benchmark(group="bloat")
@pytest.mark.parametrize(
    "payload",
    ["default", "string"] + [2**i for i in (8, 10, 12, 14, 16, 18)],
)
def test_bloat(payload, benchmark):
    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    crs = CRS.from_epsg(3857)
    layer = Layer.from_data(Layer.GT_LINESTRING, crs, (), ())
    style = Style.from_defaults(
        layer_type=LT_VECTOR,
        layer_geometry_type=Layer.GT_LINESTRING,
        color=(255, 0, 0, 255),
    )

    if payload != "default":
        qml = style.to_string()
        if payload != "string":
            inspos = len(qml) - len("</qgis>") - 1
            qml_head = qml[:inspos]
            qml_tail = qml[inspos:]
            assert qml_tail.strip() == "</qgis>"
            baloon = "<!-- baloon -->\n"
            qml = qml_head + (baloon * payload) + qml_tail
        style = Style.from_string(qml)

    def _render_image():
        mreq = MapRequest()
        mreq.set_dpi(96)
        mreq.set_crs(crs)
        mreq.add_layer(layer, style)
        mreq.render_image(extent, (512, 512))

    benchmark(_render_image)
