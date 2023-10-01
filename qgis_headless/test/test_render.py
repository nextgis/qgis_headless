import os
import os.path
from binascii import a2b_hex
from itertools import product
from tempfile import NamedTemporaryFile

import pytest
from packaging import version
from pytest import approx

from qgis_headless import (
    CRS,
    LT_VECTOR,
    Layer,
    MapRequest,
    Style,
    StyleFormat,
    StyleTypeMismatch,
    get_qgis_version,
    set_svg_paths,
)
from qgis_headless.util import (
    EXTENT_ONE,
    cmp_colors,
    image_stat,
    render_raster,
    render_vector,
    to_pil,
)

QGIS_VERSION = version.parse(get_qgis_version().split("-")[0])

WKB_MSC = a2b_hex("01010000005070B1A206CF42409CDCEF5014E04B40")  # POINT (37.61739 55.75062)
EPSG_4326_WKT = 'GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AUTHORITY["EPSG","4326"]]'
EPSG_3395_WKT = 'PROJCS["WGS 84 / World Mercator",GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]],PROJECTION["Mercator_1SP"],PARAMETER["central_meridian",0],PARAMETER["scale_factor",1],PARAMETER["false_easting",0],PARAMETER["false_northing",0],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH],AUTHORITY["EPSG","3395"]]'


def test_contour(save_img, shared_datadir, reset_svg_paths):
    data = shared_datadir / "contour/data.geojson"
    style = (shared_datadir / "contour/rgb.qml").read_text()

    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    img = save_img(render_vector(data, style, extent, 1024, svg_resolver=lambda x: x))
    stat = image_stat(img)

    assert stat.alpha.min == 0, "There are no transparent pixels found"
    assert stat.red.max == 255, "Labels aren't visible"
    assert stat.green.max == 255, "Primary lines aren't visible"
    assert stat.blue.max == 255, "Primary lines aren't visible"

    assert 3 < stat.red.mean < 4
    assert 12 < stat.green.mean < 13
    assert 29 < stat.blue.mean < 30


@pytest.mark.parametrize("format", ["qml", "sld"])
def test_format(format, save_img, shared_datadir, reset_svg_paths):
    data = shared_datadir / "contour/data.geojson"
    style = (shared_datadir / f"contour/red.{format}").read_text()

    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    img = save_img(
        render_vector(
            data,
            style,
            extent,
            1024,
            svg_resolver=lambda value: value,
            style_format=getattr(StyleFormat, format.upper()),
        )
    )
    stat = image_stat(img)

    assert stat.alpha.min == 0, "No transparent pixels found"
    assert stat.red.max == 255, "Red lines aren't visible"
    assert stat.green.max == 0, "Green channel not empty"
    assert stat.blue.max == 0, "Blue channel not empty"


def test_contour_pdf(shared_datadir, reset_svg_paths):
    layer = Layer.from_ogr(str(shared_datadir / "contour/data.geojson"))
    style = Style.from_file(str(shared_datadir / "contour/rgb.qml"))

    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(CRS.from_epsg(3857))
    req.add_layer(layer, style)

    with NamedTemporaryFile() as f:
        req.export_pdf(f.name, extent, (256, 256))
        assert os.stat(f.name).st_size > 100


def test_opacity(save_img, shared_datadir, reset_svg_paths):
    data = shared_datadir / "contour/data.geojson"
    style = (shared_datadir / "contour/opacity.qml").read_text()

    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    img = save_img(render_vector(data, style, extent, 1024))
    stat = image_stat(img)

    assert stat.alpha.min == 0, "There are no transparent pixels found"
    assert stat.alpha.max == 127, "Maximum alpha must be 127 (50%)"


def test_rule_based_labeling(save_img, shared_datadir, reset_svg_paths):
    data = shared_datadir / "contour/data.geojson"
    style = (shared_datadir / "contour/rbl.qml").read_text()

    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    img = save_img(render_vector(data, style, extent, 1024))

    stat = image_stat(img)
    assert stat.green.max == 255, "Green labels aren't visible"
    assert stat.blue.max == 255, "Blue labels aren't visible"


def test_marker_simple(save_img, shared_datadir, reset_svg_paths):
    data = shared_datadir / "zero/data.geojson"
    style = (shared_datadir / "zero/marker.qml").read_text()

    set_svg_paths([str(shared_datadir / "marker-blue")])
    img = save_img(render_vector(data, style, EXTENT_ONE, 256))

    stat = image_stat(img)
    assert stat.red.max == stat.green.max == 0, "Unexpected data in red or green channel"
    assert stat.blue.max == 255, "Blue marker is missing"


def test_marker_change(shared_datadir, reset_svg_paths):
    data = shared_datadir / "zero/data.geojson"
    style = (shared_datadir / "zero/marker.qml").read_text()

    # Render with blue marker in SVG paths
    set_svg_paths([str(shared_datadir / "marker-blue")])
    img = render_vector(data, style, EXTENT_ONE, 256)
    assert image_stat(img).blue.max == 255, "Blue marker is missing"

    # Render with green marker in SVG paths
    set_svg_paths([str(shared_datadir / "marker-green")])
    img = render_vector(data, style, EXTENT_ONE, 256)
    assert image_stat(img).green.max == 255, "Green marker is missing"


@pytest.mark.skipif(
    not os.path.exists("/usr/share/qgis/svg"),
    reason="Builtin SVG icons are missing",
)
def test_svg_builtin(shared_datadir, reset_svg_paths):
    data = shared_datadir / "zero/data.geojson"
    style = (shared_datadir / "zero/builtin.qml").read_text()

    set_svg_paths(["/usr/share/qgis/svg"])

    layer = Layer.from_ogr(str(data))
    style = Style.from_string(style)

    img = render_vector(layer, style, EXTENT_ONE, 256)
    assert image_stat(img).red.max == 255, "Red marker is missing"


@pytest.mark.parametrize(
    "icon, svg_resolver",
    [
        pytest.param("rect", None, id="rect-none"),
        pytest.param("camera", None, id="camera-none"),
        pytest.param("rect", lambda p: p, id="rect-dummy"),
        pytest.param("camera", lambda p: p, id="camera-dummy"),
    ],
)
def test_svg_colorize(icon, svg_resolver, save_img, shared_datadir, reset_svg_paths):
    layer = shared_datadir / "zero/data.geojson"
    style = (shared_datadir / f"zero/svg-colorize-{icon}.qml").read_text()

    img = save_img(render_vector(layer, style, EXTENT_ONE, 256, svg_resolver=svg_resolver))
    assert image_stat(img).red.max == 255, "Red fill is missing"
    assert image_stat(img).green.max == 255, "Green outline is missing"


def test_svg_resolver(shared_datadir, reset_svg_paths):
    data = shared_datadir / "zero/data.geojson"
    style_marker = (shared_datadir / "zero/marker.qml").read_text()

    color = None
    resolved = list()

    def _resolver(source):
        target = str((shared_datadir / "marker-{}".format(color) / source).resolve())
        resolved.append(source)
        return target

    color = "blue"
    resolved.clear()

    img = render_vector(data, style_marker, EXTENT_ONE, 256, svg_resolver=_resolver)
    assert resolved == ["marker.svg"], "Marker isn't resolved"
    assert image_stat(img).blue.max == 255, "Blue marker is missing"

    color = "green"
    resolved.clear()

    img = render_vector(data, style_marker, EXTENT_ONE, 256, svg_resolver=_resolver)
    assert resolved == ["marker.svg"], "Marker isn't resolved"
    assert image_stat(img).green.max == 255, "Green marker is missing"

    color = "missing"
    resolved.clear()

    img = render_vector(data, style_marker, EXTENT_ONE, 256, svg_resolver=_resolver)

    stat = image_stat(img)
    assert resolved == ["marker.svg"], "Marker isn't resolved"
    assert stat.red.min == stat.green.min == stat.blue.min == 0, "Black question mark is missing"

    style_svg_fill = (shared_datadir / "zero/svg-fill.qml").read_text()

    color = "blue"
    resolved.clear()

    img = render_vector(data, style_svg_fill, EXTENT_ONE, 256, svg_resolver=_resolver)
    assert resolved == ["marker.svg"], "Marker isn't resolved"
    assert image_stat(img).blue.max == 255, "Blue marker is missing"


@pytest.mark.skipif(
    QGIS_VERSION < version.parse("3.14"),
    reason="Fetching marker by URL may fail in QGIS < 3.14",
)
def test_marker_url(save_img, shared_datadir, reset_svg_paths, capfd):
    data = shared_datadir / "zero/data.geojson"
    style = (shared_datadir / "zero/marker-url.qml").read_text()

    img = save_img(render_vector(data, style, EXTENT_ONE, 256))

    assert capfd.readouterr().out.strip() == "", "QGIS stdout output was captured"
    assert capfd.readouterr().err.strip() == "", "QGIS stderr output was captured"

    stat = image_stat(img)
    assert stat.red.max == 255 and stat.green.max == stat.blue.max == 0, "Red marker is missing"


def test_svg_cache(shared_datadir, reset_svg_paths):
    data = shared_datadir / "zero/data.geojson"
    style = (shared_datadir / "zero/marker.qml").read_text()
    marker = (shared_datadir / "marker-blue" / "marker.svg").resolve()

    layer = Layer.from_ogr(str(data))
    style = Style.from_string(style, svg_resolver=lambda _: str(marker))

    req = MapRequest()
    req.set_crs(CRS.from_epsg(3857))
    req.add_layer(layer, style)

    rendered_image = req.render_image(EXTENT_ONE, (256, 256))
    img = to_pil(rendered_image)
    assert image_stat(img).blue.max == 255, "Blue marker is missing"

    marker.unlink()  # Remove marker file from directory

    # And render again
    rendered_image = req.render_image(EXTENT_ONE, (256, 256))
    img = to_pil(rendered_image)
    assert image_stat(img).blue.max == 255, "Marker is missing in same MapRequest"

    # Recreata MapRequest with same Layer and Style
    req = MapRequest()
    req.set_crs(CRS.from_epsg(3857))
    req.add_layer(layer, style)

    # And render again
    rendered_image = req.render_image(EXTENT_ONE, (256, 256))
    img = to_pil(rendered_image)
    assert image_stat(img).blue.max == 255, "Marker is missing in new MapRequest"


def test_legend(save_img, shared_datadir, reset_svg_paths):
    data = shared_datadir / "contour/data.geojson"
    style = (shared_datadir / "contour/rgb.qml").read_text()

    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(CRS.from_epsg(3857))

    req.add_layer(Layer.from_ogr(str(data)), Style.from_string(style), label="Contour")

    rendered_legend = req.render_legend()
    img = save_img(to_pil(rendered_legend))

    assert img.size == (223, 92), "Expected size is 223 x 92"

    stat = image_stat(img)
    assert stat.green.max == 255, "Primary lines aren't visible"
    assert stat.blue.max == 255, "Primary lines aren't visible"
    assert stat.red.mean == 0
    assert 1 < stat.green.mean < 3
    assert 1 < stat.blue.mean < 3

    req.set_dpi(2 * 96)
    rendered_legend = req.render_legend()
    hdpi_img = to_pil(rendered_legend)

    assert (
        img.size[0] < hdpi_img.size[0] and img.size[1] < hdpi_img.size[1]
    ), "Higher DPI should produce bigger legend"


RED = (255, 0, 0, 255)
GREEN = (0, 255, 0, 255)
BLUE = (0, 0, 255, 255)
TRANSPARENT = (0, 0, 0, 0)
GRAY_128 = (128, 128, 128, 128)

LEGEND_DEFAULT_SIZES = [20, 40]

legend_symbols_params = []
for id, gt, style, sizes, expected in [
    (
        "zero_red_circle",
        Layer.GT_POINT,
        dict(file="zero/red-circle.qml"),
        LEGEND_DEFAULT_SIZES + [24, 32, (32, 24)],
        [
            (None, dict({k: TRANSPARENT for k in ("lt", "rt", "lb", "rb")}, c=RED)),
        ],
    ),
    (
        "default_point",
        Layer.GT_POINT,
        dict(color=GRAY_128),
        LEGEND_DEFAULT_SIZES,
        ((None, dict({k: TRANSPARENT for k in ("lt", "rt", "lb", "rb")}, c=GRAY_128)),),
    ),
    (
        "default_linestring",
        Layer.GT_LINESTRING,
        dict(color=GRAY_128),
        LEGEND_DEFAULT_SIZES,
        ((None, dict(ct=TRANSPARENT, cb=TRANSPARENT)),),
    ),
    (
        "default_polygon",
        Layer.GT_POLYGON,
        dict(color=GRAY_128),
        LEGEND_DEFAULT_SIZES,
        ((None, GRAY_128),),
    ),
    (
        "contour_rgb",
        Layer.GT_LINESTRING,
        dict(file="contour/rgb.qml"),
        LEGEND_DEFAULT_SIZES + [24, 32, (32, 24)],
        (
            ("primary horizontals", dict(c=GREEN, ct=TRANSPARENT, cb=TRANSPARENT)),
            ("secondary horizontals", dict(c=BLUE, ct=TRANSPARENT, cb=TRANSPARENT)),
        ),
    ),
    (
        "thick_outline",
        Layer.GT_POLYGON,
        dict(file="thick-outline.qml"),
        LEGEND_DEFAULT_SIZES,
        ((None, dict()),),
    ),
    (
        "big_symbol",
        Layer.GT_POINT,
        dict(file="big-symbol.qml"),
        LEGEND_DEFAULT_SIZES,
        ((None, dict()),),
    ),
]:
    for size in sizes:
        if type(size) == int:
            size = (size, size)
        param_id = f"{id}-{size[0]}x{size[1]}"
        legend_symbols_params.append(pytest.param(gt, style, size, expected, id=param_id))


@pytest.mark.parametrize("gt, style_params, size, expected", legend_symbols_params)
def test_legend_symbols(gt, style_params, size, expected, save_img, shared_datadir):
    if "file" in style_params:
        style = Style.from_file(str(shared_datadir / style_params["file"]))
    else:
        style = Style.from_defaults(
            layer_type=LT_VECTOR,
            layer_geometry_type=gt,
            color=style_params["color"],
        )

    req = MapRequest()
    req.set_dpi(96)
    req.add_layer(Layer.from_data(gt, CRS.from_epsg(3857), (), ()), style)

    symbols = req.legend_symbols(0, size)
    symbols_count = len(symbols)
    expected_count = len(expected)
    assert symbols_count == expected_count, "count mismatch"

    for symbol, (title, color) in zip(symbols, expected):
        assert symbol.title() == title, "title mismatch"

        image = save_img(to_pil(symbol.icon()), symbol.title())
        im_size = image.size

        im_width, im_height = im_size
        assert im_width != 0, "zero width"
        assert im_height != 0, "zero height"

        pixel_coord = (
            dict(l=0, r=im_width - 1, c=(im_width - 1) // 2),
            dict(t=0, b=im_height - 1, c=(im_height - 1) // 2),
        )

        if isinstance(color, tuple):
            color = dict(c=color)

        for k, v in color.items():
            if k == "c":
                k = "cc"
            p = tuple(b[i] for i, b in zip(k, pixel_coord))
            color = image.getpixel(p)
            delta = cmp_colors(color, v)
            assert delta < 20, f"{k}{p} color mismatch ({delta}): {color} != {v} "

        assert im_size == size, f"size mismatch: {im_size} != {size}"


def test_legend_svg_path(save_img, shared_datadir, reset_svg_paths):
    data = shared_datadir / "zero/data.geojson"
    style = (shared_datadir / "zero/marker.qml").read_text()

    set_svg_paths([str(shared_datadir / "marker-blue")])

    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(CRS.from_epsg(3857))

    req.add_layer(Layer.from_ogr(str(data)), Style.from_string(style), label="Marker")

    rendered_legend = req.render_legend()
    img = save_img(to_pil(rendered_legend))

    stat = image_stat(img)
    assert stat.blue.max == 255, "Blue marker is missing"


def test_legend_svg_resolver(save_img, shared_datadir, reset_svg_paths):
    data = shared_datadir / "zero/data.geojson"
    style = (shared_datadir / "zero/marker.qml").read_text()
    marker = (shared_datadir / "marker-blue" / "marker.svg").resolve()

    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(CRS.from_epsg(3857))

    req.add_layer(
        Layer.from_ogr(str(data)),
        Style.from_string(style, svg_resolver=lambda _: str(marker)),
        label="Marker",
    )

    rendered_legend = req.render_legend()
    img = save_img(to_pil(rendered_legend))

    stat = image_stat(img)
    assert stat.blue.max == 255, "Blue marker is missing"


@pytest.mark.parametrize(
    "crs, extent, extent_empty",
    (
        pytest.param(
            CRS.from_epsg(4326),
            (37.60, 55.74, 37.62, 55.76),
            (37.60, 57.74, 37.62, 57.76),
            id="from EPSG:4326",
        ),
        pytest.param(
            CRS.from_wkt(EPSG_4326_WKT),
            (37.60, 55.74, 37.62, 55.76),
            (37.60, 57.74, 37.62, 57.76),
            id="from WKT EPSG:4326",
        ),
        pytest.param(
            CRS.from_epsg(3857),
            (4187547.0, 7508930.0, 4187549.0, 7508932.0),
            (4187547.0, 7473582.0, 4187549.0, 7473584.0),
            id="from EPSG:3857",
        ),
        pytest.param(
            CRS.from_wkt(EPSG_3395_WKT),
            (4187547.0, 7473582.0, 4187549.0, 7473584.0),
            (4187547.0, 7508930.0, 4187549.0, 7508932.0),
            id="from WKT EPSG:3395",
        ),
    ),
)
def test_render_crs(crs, extent, extent_empty, save_img, shared_datadir):
    source_crs = CRS.from_epsg(4326)
    layer = Layer.from_data(Layer.GT_POINT, source_crs, tuple(), ((1, WKB_MSC, tuple()),))

    style = (shared_datadir / "zero/red-circle.qml").read_text()

    img = save_img(render_vector(layer, style, extent, 1024, crs=crs), "vector")
    stat = image_stat(img)
    assert stat.green.max == stat.blue.max == 0, "Unexpected data in blue or green channel"
    assert stat.red.max == 255, "Red marker is missing"

    img_empty = save_img(render_vector(layer, style, extent_empty, 256, crs=crs), "raster")
    stat = image_stat(img_empty)
    assert stat.red.max == stat.green.max == stat.blue.max == 0, "Unexpected non-empty image"


def test_attribute_color(save_img, shared_datadir):
    data = shared_datadir / "landuse" / "landuse.geojson"
    layer = Layer.from_ogr(str(data))

    style = (shared_datadir / "landuse" / "landuse.qml").read_text()

    img = save_img(
        render_vector(
            layer,
            style,
            (4189314.0, 7505071.0, 4190452.0, 7506101.0),
            svg_resolver=lambda x: x,
        )
    )

    stat = image_stat(img)

    assert stat.red.max == 255, "Red polygon is missing"
    assert stat.green.max == 255, "Green polygon is missing"
    assert stat.blue.max == 0, "Blue band is not expected"


def test_style_25d(save_img, shared_datadir):
    data = shared_datadir / "poly.geojson"
    layer = Layer.from_ogr(str(data))

    style = (shared_datadir / "25d" / "poly_25d.qml").read_text()

    img = save_img(render_vector(layer, style, (-10, -10, 10, 10)))

    stat = image_stat(img)

    assert stat.red.max == 255, "Shadow is missing"
    assert stat.blue.max == 255, "Roof is missing"
    assert stat.green.max == approx(255, abs=1), "Walls are missing"


@pytest.mark.parametrize(
    "layer",
    (
        pytest.param("diagram/industries.geojson", id="industries"),
        pytest.param("diagram/industries-copy.geojson", id="industries-copy"),
    ),
)
def test_diagram(layer, save_img, shared_datadir):
    style = Style.from_file(str(shared_datadir / "diagram" / "industries.qml"))
    layer = Layer.from_ogr(str(shared_datadir / layer))

    img = save_img(
        render_vector(
            layer, style, (33.86681, 45.05880, 33.86878, 45.06046), crs=CRS.from_epsg(4326)
        )
    )

    assert img.getpixel((70, 138)) == (254, 221, 74, 255)
    assert img.getpixel((111, 135)) == (41, 187, 255, 255)
    assert img.getpixel((96, 173)) == (204, 97, 20, 255)

    # layer = Layer.from_data(
    #     Layer.GT_POINT, CRS.from_epsg(4326), (
    #         ('zern', Layer.FT_INTEGER),
    #         ('ovosch', Layer.FT_INTEGER),
    #         ('sad', Layer.FT_INTEGER),
    #         ('vinograd', Layer.FT_INTEGER),
    #         ('efir', Layer.FT_INTEGER),
    #         ('skotovod', Layer.FT_INTEGER),
    #         ('svinovod', Layer.FT_INTEGER),
    #         ('svinovod', Layer.FT_INTEGER),
    #         ('total', Layer.FT_INTEGER),
    #     ), (
    #         (1, WKB_POINT_00, (1, 1, 1, 1, 0, 0, 0, 0, 4)),
    #     )
    # )

    # img = render_vector(layer, style, EXTENT_ONE, crs=CRS.from_epsg(3857))
    # img.save('share/from_data.png')

    # layer = Layer.from_ogr(str(shared_datadir / 'diagram' / 'ind.geojson'))
    # img = render_vector(layer, style, EXTENT_ONE, crs=CRS.from_epsg(3857))
    # img.save('share/from_ogr.png')


def test_gradient(save_img, shared_datadir):
    style = Style.from_file(str(shared_datadir / "gradient.qml"))
    layer = Layer.from_ogr(str(shared_datadir / "landuse/landuse.geojson"))

    img = save_img(render_vector(layer, style, (4189625, 7505162, 4190004, 7506081)))
    stat = image_stat(img)

    assert stat.red.max == 255, "First colour is missing"
    assert stat.green.max == 255, "Middle colour is missing"
    assert stat.blue.max == 255, "Last colour is missing"


def test_raster(shared_datadir):
    layer = Layer.from_gdal(str(shared_datadir / "raster" / "rounds.tif"))
    style = Style.from_file(str(shared_datadir / "raster" / "rounds.qml"))

    img = render_raster(layer, style, (251440.0, 5977974.0, 1978853.0, 7505647.0))
    stat = image_stat(img)
    assert (stat.red.max, stat.green.max, stat.blue.max) == (255, 0, 0), "Red colour missing"

    img = render_raster(layer, style, (3848936.0, 5977974.0, 5503915.0, 7505647.0))
    stat = image_stat(img)
    assert (stat.red.max, stat.green.max, stat.blue.max) == (0, 255, 0), "Green colour missing"

    img = render_raster(layer, style, (251440.0, 2556073.0, 1978853.0, 4158374.0))
    stat = image_stat(img)
    assert (stat.red.max, stat.green.max, stat.blue.max) == (0, 0, 255), "Blue colour missing"

    img = render_raster(layer, style, (3848936.0, 2556073.0, 5503915.0, 4158374.0))
    stat = image_stat(img)
    assert (stat.red.max, stat.green.max, stat.blue.max) == (0, 0, 0), "Black colour missing"


def test_raster_layer_vector_style(shared_datadir):
    layer = Layer.from_gdal(str(shared_datadir / "raster" / "rounds.tif"))
    style = Style.from_file(str(shared_datadir / "point-style.qml"))

    mreq = MapRequest()
    with pytest.raises(StyleTypeMismatch):
        mreq.add_layer(layer, style)


def test_vector_layer_raster_style(shared_datadir):
    layer = Layer.from_ogr(str(shared_datadir / "poly.geojson"))
    style = Style.from_file(str(shared_datadir / "raster" / "rounds.qml"))

    mreq = MapRequest()
    with pytest.raises(StyleTypeMismatch):
        mreq.add_layer(layer, style)


@pytest.mark.parametrize(
    "data, color",
    [
        pytest.param(data, color, id=f"{data}-{color}")
        for data, color in product(
            ["zero/data.geojson", "line.geojson", "poly.geojson"],
            (
                (0, 0, 0, 0),
                (0, 0, 0, 255),
                (70, 180, 150, 255),
                (42, 24, 33, 180),
            ),
        )
    ],
)
def test_vector_default_style(data, color, save_img, shared_datadir):
    data = shared_datadir / data
    style = Style.from_defaults(color=color)

    size = 64

    # QGIS uses length units (not pixels!) for line styles by default. For 96
    # dpi (default) antialiasing blurs lines and there are no solid filled
    # pixels there. That's why setting a higher resolution does the trick and
    # makes lines thicker.

    img = save_img(render_vector(data, style, EXTENT_ONE, size, dpi=600))
    pick_color = img.getpixel((size // 2, size // 2))

    for i, band in enumerate(("red", "green", "blue", "alpha")):
        assert approx(pick_color[i], abs=1) == color[i], f"{band.capitalize()} band value mismatch"


def test_raster_rgb_default_style(shared_datadir):
    layer = Layer.from_gdal(str(shared_datadir / "raster/rounds.tif"))
    style = Style.from_defaults()

    stat = image_stat(render_raster(layer, style, (251440.0, 5977974.0, 1978853.0, 7505647.0)))
    assert (stat.red.max, stat.green.max, stat.blue.max) == (255, 0, 0), "Red colour missing"

    stat = image_stat(render_raster(layer, style, (3848936.0, 5977974.0, 5503915.0, 7505647.0)))
    assert (stat.red.max, stat.green.max, stat.blue.max) == (0, 255, 0), "Green colour missing"

    stat = image_stat(render_raster(layer, style, (251440.0, 2556073.0, 1978853.0, 4158374.0)))
    assert (stat.red.max, stat.green.max, stat.blue.max) == (0, 0, 255), "Blue colour missing"

    stat = image_stat(render_raster(layer, style, (3848936.0, 2556073.0, 5503915.0, 4158374.0)))
    assert (stat.red.max, stat.green.max, stat.blue.max) == (0, 0, 0), "Black colour missing"


def test_raster_rgb_inverted_style(shared_datadir):
    layer = Layer.from_gdal(str(shared_datadir / "raster/rounds.tif"))
    style = Style.from_file(str(shared_datadir / "raster/inverted.qml"))

    stat = image_stat(render_raster(layer, style, (251440.0, 5977974.0, 1978853.0, 7505647.0)))
    assert (stat.red.max, stat.green.max, stat.blue.max) == (0, 255, 255), "Cyan colour missing"

    stat = image_stat(render_raster(layer, style, (3848936.0, 5977974.0, 5503915.0, 7505647.0)))
    assert (stat.red.max, stat.green.max, stat.blue.max) == (255, 0, 255), "Magenta colour missing"

    stat = image_stat(render_raster(layer, style, (251440.0, 2556073.0, 1978853.0, 4158374.0)))
    assert (stat.red.max, stat.green.max, stat.blue.max) == (255, 255, 0), "Yellow colour missing"

    stat = image_stat(render_raster(layer, style, (3848936.0, 2556073.0, 5503915.0, 4158374.0)))
    assert (stat.red.max, stat.green.max, stat.blue.max) == (255, 255, 255), "White colour missing"


def test_raster_dem_default_style(save_img, shared_datadir):
    layer = Layer.from_gdal(str(shared_datadir / "raster/sochi-aster-dem.tif"))
    style = Style.from_defaults()

    img = save_img(render_raster(layer, style, (40.0, 43.0, 41.0, 44.0), crs=CRS.from_epsg(4326)))
    stat = image_stat(img)

    assert stat.alpha.min == stat.alpha.max == 255
    assert stat.red.min == stat.green.min == stat.blue.min, "Bands min not equal"
    assert stat.red.max == stat.green.max == stat.blue.max, "Bands max not equal"

    assert approx(stat.red.mean) == approx(stat.green.mean) == approx(stat.blue.mean)


def test_label_variables(save_img, shared_datadir):
    layer = Layer.from_ogr(str(shared_datadir / "zero/data.geojson"))
    style = Style.from_file(str(shared_datadir / "zero/label/variables.qml"))

    img = save_img(render_vector(layer, style, EXTENT_ONE))

    stat = image_stat(img)

    assert stat.red.max == 255, "Point is missing"
    assert stat.green.max == 255, "Map unit label is missing"
    assert stat.blue.max == 255, "Map scale label is missing"


@pytest.mark.parametrize(
    "qml, resolve",
    [
        pytest.param("zero/label/marker-svg.qml", True, id="svg"),
        pytest.param("zero/label/marker-symbol-svg.qml", True, id="symbol_svg"),
        pytest.param("zero/label/marker-symbol-embedded.qml", False, id="symbol_embedded"),
    ],
)
def test_label_marker(qml, resolve, save_img, shared_datadir):
    layer = shared_datadir / "zero/data.geojson"
    style = (shared_datadir / qml).read_text()

    resolved = list()

    def _resolver(source):
        target = str((shared_datadir / "marker-green" / source).resolve())
        resolved.append(source)
        return target

    img = save_img(render_vector(layer, style, EXTENT_ONE, svg_resolver=_resolver))

    assert not resolve or resolved == ["marker.svg"], "Marker isn't resolved"

    stat = image_stat(img)

    assert stat.red.max == 255, "Point is missing"
    assert stat.green.max == 255, "Label marker is missing"
