from copy import deepcopy
from pathlib import Path
from xml.etree import ElementTree

import pytest

from qgis_headless import CRS, Layer, MapRequest, Style
from qgis_headless.util import GREEN, RED, to_pil

from .known_issues import Issues


@pytest.fixture
def marker_style(shared_datadir: Path) -> ElementTree.Element:
    return ElementTree.fromstring((shared_datadir / "zero/red-circle.qml").read_text())


def _element(root: ElementTree.Element, path: str) -> ElementTree.Element:
    element = root.find(path)
    assert element is not None, f"Missing style element: {path}"
    return element


def _set_properties(layer: ElementTree.Element, **properties: str) -> None:
    for name, value in properties.items():
        element = layer.find(f"prop[@k='{name}']")
        if element is None:
            element = ElementTree.SubElement(layer, "prop", k=name)
        element.set("v", value)


def _set_expression(element: ElementTree.Element, name: str, expression: str) -> None:
    collection = element.find("data_defined_properties/Option")
    if collection is None:
        properties = ElementTree.SubElement(element, "data_defined_properties")
        collection = ElementTree.SubElement(properties, "Option", type="Map")
        ElementTree.SubElement(collection, "Option", name="name", type="QString", value="")
        ElementTree.SubElement(collection, "Option", name="properties")
        ElementTree.SubElement(
            collection, "Option", name="type", type="QString", value="collection"
        )
    properties = _element(collection, "Option[@name='properties']")
    properties.set("type", "Map")
    entry = ElementTree.SubElement(properties, "Option", name=name, type="Map")
    ElementTree.SubElement(entry, "Option", name="active", type="bool", value="true")
    ElementTree.SubElement(entry, "Option", name="expression", type="QString", value=expression)
    ElementTree.SubElement(entry, "Option", name="type", type="int", value="3")


def _request(root: ElementTree.Element) -> MapRequest:
    request = MapRequest()
    request.set_dpi(96)
    request.add_layer(
        Layer.from_data(Layer.GT_POINT, CRS.from_epsg(3857), (), ()),
        Style.from_string(ElementTree.tostring(root, encoding="unicode")),
        label="legend-layer",
    )
    return request


@pytest.mark.parametrize("opacity, expected_alpha", [(0, 0), (0.5, 128), (1, 255)])
def test_symbol_opacity(marker_style, opacity, expected_alpha):
    _element(marker_style, "renderer-v2/symbols/symbol").set("alpha", str(opacity))
    image = to_pil(_request(marker_style).legend_symbols(0, (40, 40))[0].icon())
    assert abs(image.getchannel("A").getextrema()[1] - expected_alpha) <= 1


@pytest.mark.parametrize(
    "opacity, expected_alpha",
    [
        pytest.param(0, 0, marks=Issues.LEGEND_LAYER_OPACITY),
        pytest.param(0.5, 128, marks=Issues.LEGEND_LAYER_OPACITY),
        (1, 255),
    ],
)
def test_layer_opacity(marker_style, opacity, expected_alpha):
    ElementTree.SubElement(marker_style, "layerOpacity").text = str(opacity)
    image = to_pil(_request(marker_style).legend_symbols(0, (40, 40))[0].icon())
    assert abs(image.getchannel("A").getextrema()[1] - expected_alpha) <= 1


@Issues.LEGEND_SYMBOL_CLIPPING
@pytest.mark.parametrize("unit, size", [("Pixel", "40"), ("MM", "40"), ("MapUnit", "1000")])
def test_large_marker_fits_icon(marker_style, unit, size):
    _set_properties(
        _element(marker_style, "renderer-v2/symbols/symbol/layer"), size=size, size_unit=unit
    )
    image = to_pil(_request(marker_style).legend_symbols(0, (20, 20))[0].icon())
    bounds = image.getchannel("A").getbbox()
    assert bounds is not None, "Large marker is invisible"
    left, top, right, bottom = bounds
    assert 0 < left < right < 20 and 0 < top < bottom < 20, "Large marker is clipped"


@Issues.LEGEND_SYMBOL_CLIPPING
def test_large_star_fits_icon(shared_datadir):
    request = MapRequest()
    request.set_dpi(96)
    request.add_layer(
        Layer.from_data(Layer.GT_POINT, CRS.from_epsg(3857), (), ()),
        Style.from_file(shared_datadir / "big-symbol.qml"),
    )
    image = to_pil(request.legend_symbols(0, (20, 20))[0].icon())
    bounds = image.getchannel("A").getbbox()
    assert bounds is not None, "Star outline is invisible"
    left, top, right, bottom = bounds
    assert 0 < left < right < 20 and 0 < top < bottom < 20, "Star outline is clipped"


@pytest.mark.parametrize(
    "expression",
    [
        pytest.param("'0,255,0,255'", id="constant"),
        pytest.param(
            "if(@layer_name = 'legend-layer', '0,255,0,255', '255,0,0,255')",
            id="layer-variable",
            marks=Issues.LEGEND_EXPRESSION_SCOPE,
        ),
        pytest.param(
            "if(@qgis_version IS NOT NULL, '0,255,0,255', '255,0,0,255')",
            id="global-variable",
            marks=Issues.LEGEND_EXPRESSION_SCOPE,
        ),
    ],
)
def test_expression_color(marker_style, expression):
    _set_expression(
        _element(marker_style, "renderer-v2/symbols/symbol/layer"), "fillColor", expression
    )
    image = to_pil(_request(marker_style).legend_symbols(0, (40, 40))[0].icon())
    assert image.getpixel((20, 20)) == GREEN


@Issues.LEGEND_EXPRESSION_OPACITY
def test_expression_opacity(marker_style):
    _set_expression(_element(marker_style, "renderer-v2/symbols/symbol"), "alpha", "25")
    image = to_pil(_request(marker_style).legend_symbols(0, (40, 40))[0].icon())
    assert abs(image.getchannel("A").getextrema()[1] - 64) <= 1


@Issues.LEGEND_COLLAPSED_SIZE
def test_collapsed_size_legend(marker_style):
    _set_expression(_element(marker_style, "renderer-v2/symbols/symbol/layer"), "size", "4")
    renderer = _element(marker_style, "renderer-v2")
    legend = ElementTree.SubElement(
        renderer, "data-defined-size-legend", type="collapsed", title="Sizes", valign="bottom"
    )
    classes = ElementTree.SubElement(legend, "classes")
    for size, label in [(4, "small"), (8, "medium"), (16, "large")]:
        ElementTree.SubElement(classes, "class", size=str(size), label=label)
    symbols = _request(marker_style).legend_symbols(0, (64, 64))
    assert len(symbols) == 2, "Expected a size-legend heading and collapsed chart"
    icon = symbols[1].icon()
    assert icon.size() == (64, 64), "Collapsed size legend has no image"
    assert to_pil(icon).getchannel("A").getbbox() is not None


@pytest.mark.parametrize("size", [(1, 1), (3, 5), (17, 11), (31, 25)])
@pytest.mark.parametrize("dpi", [96, 192, 384])
def test_small_and_odd_icon_sizes(marker_style, size, dpi):
    request = _request(marker_style)
    request.set_dpi(dpi)
    image = to_pil(request.legend_symbols(0, size)[0].icon())
    assert image.size == size
    assert image.getchannel("A").getbbox() is not None, "Marker vanished in a small icon"
    assert image.getpixel((size[0] // 2, size[1] // 2)) == RED


@pytest.mark.parametrize("kind", ["svg", "font", "multilayer", "geometry-generator"])
@pytest.mark.parametrize("dpi", [96, 192, 384])
def test_complex_marker_visible(marker_style, kind, dpi, tmp_path):
    symbol = _element(marker_style, "renderer-v2/symbols/symbol")
    layer = _element(symbol, "layer")
    if kind == "multilayer":
        foreground = deepcopy(layer)
        _set_properties(foreground, color="0,255,0,255", size="6")
        symbol.append(foreground)
    elif kind == "geometry-generator":
        nested_symbol = deepcopy(symbol)
        nested_symbol.set("name", "@0@0")
        layer.clear()
        layer.set("class", "GeometryGenerator")
        _set_properties(layer, geometryModifier="$geometry", SymbolType="Marker", units="MapUnit")
        layer.append(nested_symbol)
    else:
        layer.clear()
        if kind == "svg":
            path = tmp_path / "marker.svg"
            path.write_text(
                '<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20">'
                '<rect width="20" height="20" fill="red"/></svg>'
            )
            layer.set("class", "SvgMarker")
            _set_properties(layer, name=str(path), size="16", size_unit="Pixel")
        else:
            layer.set("class", "FontMarker")
            _set_properties(
                layer,
                font="DejaVu Sans",
                chr="A",
                color="255,0,0,255",
                size="16",
                size_unit="Pixel",
            )
    request = _request(marker_style)
    request.set_dpi(dpi)
    image = to_pil(request.legend_symbols(0, (40, 40))[0].icon())
    assert image.size == (40, 40)
    assert image.getchannel("A").getbbox() is not None, f"{kind} marker is invisible"
    assert image.getchannel("R").getextrema()[1] == 255, f"{kind} marker lost its red component"
    if kind == "multilayer":
        assert image.getpixel((20, 20)) == GREEN, "Foreground symbol layer is missing"
