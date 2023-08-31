from contextlib import suppress
from io import StringIO

import pytest
from lxml import etree
from packaging import version
from pytest import param

from qgis_headless import (
    LT_RASTER,
    LT_UNKNOWN,
    LT_VECTOR,
    SF_QML,
    SF_SLD,
    Layer,
    Style,
    StyleTypeMismatch,
    StyleValidationError,
    get_qgis_version,
)

QGIS_VERSION = version.parse(get_qgis_version().split('-')[0])
QGIS_312 = QGIS_VERSION >= version.parse('3.12')


def test_non_existent_file(shared_datadir):
    with pytest.raises(StyleValidationError):
        Style.from_file(str(shared_datadir / 'missing.qml'))


def test_invalid_file(shared_datadir):
    with pytest.raises(StyleValidationError):
        Style.from_file(str(shared_datadir / 'invalid.qml'))


def test_empty_string(shared_datadir):
    with pytest.raises(StyleValidationError):
        Style.from_string('')


@pytest.mark.parametrize('params', (
    dict(layer_type=LT_RASTER),
    dict(layer_type=LT_VECTOR, layer_geometry_type=Layer.GT_POINT, color=(14, 15, 16, 250)),
    dict(layer_type=LT_VECTOR, layer_geometry_type=Layer.GT_LINESTRING, color=(7, 6, 5, 4)),
    dict(layer_type=LT_VECTOR, layer_geometry_type=Layer.GT_POLYGON, color=(7, 6, 5, 4)),
))
def test_default(params):
    style = Style.from_defaults(**params)
    qml = style.to_string()

    root = etree.parse(StringIO(qml), parser=etree.XMLParser()).getroot()

    assert root.tag == 'qgis'

    if color := params.get('color'):
        gt = params['layer_geometry_type']
        if gt in (Layer.GT_POINT, Layer.GT_POLYGON):
            attr = 'color'
        elif gt == Layer.GT_LINESTRING:
            attr = 'line_color'
        else:
            raise NotImplementedError()

        search = f'''//Option[@name='{attr}']'''
        result = root.xpath(search)
        assert len(result) == 1, "Color option missing"

        qml_color = tuple(map(int, result[0].attrib['value'].split(',')))
        assert qml_color == color


@pytest.mark.parametrize("file, expected", (
    param("contour-simple.qml", [], id="contour-simple"),
    param("contour-rgb.qml", ["level"], id="contour-rgb"),
    param("contour-rbl.qml", ["level"] if QGIS_312 else None, id="contour-rbl"),
    param("boston/highway.qml", ["HIGHWAY"], id="boston-highway-qml"),
    param("boston/highway.sld", ["HIGHWAY"], id="boston-highway-sld"),
    param("attributes/default.qml", [], id="default"),
    param("attributes/osm-highway.qml", ["HIGHWAY", "NAME_EN", "NAME"], id="osm-highway"),
    param("attributes/data-defined.qml", ["size"], id="data-defined"),
    param("attributes/rule-based-labeling.qml", ["a", "b", "c"] if QGIS_312 else None, id="rule-based-labeling"),
    param(
        "diagram/industries.qml",
        ["zern", "ovosch", "sad", "vinograd", "efir", "skotovod", "svinovod", "ptitcevod", "total"],
        id="diagram", marks=pytest.mark.xfail(reason="https://github.com/qgis/QGIS/issues/33810"),
    ),
))
def test_attributes(file, expected, shared_datadir):
    if file.endswith(".qml"):
        format = SF_QML
    elif file.endswith(".sld"):
        format = SF_SLD
    else:
        raise ValueError

    style = Style.from_file(str(shared_datadir / file), format=format)
    assert style.used_attributes() == (set(expected) if expected is not None else None)



def test_attributes_default():
    style = Style.from_defaults()
    attrs = style.used_attributes()
    assert attrs == set()


@pytest.mark.parametrize("style_fn, expected", (
    param("scale/100_10.qml", (100000, 10000), id="100-10"),
    param("scale/min_50.qml", (50000, None), id="min-50"),
    param("scale/max_50.qml", (None, 50000), id="max-50"),
    param(None, (None, None), id="default"),
))
def test_scale_range(style_fn, expected, shared_datadir):
    if style_fn is None:
        style = Style.from_defaults()
    else:
        style = Style.from_file(str(shared_datadir / style_fn))

    assert style.scale_range() == expected


@pytest.mark.parametrize('style, layer_type, exc', (
    ('point-style.qml', LT_UNKNOWN, None),
    ('point-style.qml', LT_VECTOR, None),
    ('point-style.qml', LT_RASTER, StyleTypeMismatch),
    ('raster/rounds.qml', LT_UNKNOWN, None),
    ('raster/rounds.qml', LT_VECTOR, StyleTypeMismatch),
    ('raster/rounds.qml', LT_RASTER, None),
))
def test_layer_type(style, layer_type, exc, shared_datadir):
    with pytest.raises(exc) if exc is not None else suppress():
        params = dict()
        if layer_type is not None:
            params['layer_type'] = layer_type
        Style.from_file(str(shared_datadir / style), **params)


def test_geom_type(shared_datadir):
    point_style = str(shared_datadir / 'point-style.qml')
    Style.from_file(point_style)
    Style.from_file(point_style, layer_geometry_type=Layer.GT_POINT)
    with pytest.raises(StyleTypeMismatch):
        Style.from_file(point_style, layer_geometry_type=Layer.GT_POLYGON)


@pytest.mark.parametrize('style, fmt, exc', (
    ('contour-red.qml', SF_QML, None),
    ('contour-red.qml', SF_SLD, StyleValidationError),
    ('contour-red.sld', SF_SLD, None),
    ('contour-red.sld', SF_QML, StyleValidationError),
))
def test_format(style, fmt, exc, shared_datadir):
    style_file = shared_datadir / style
    with pytest.raises(exc) if exc is not None else suppress():
        Style.from_file(str(style_file), format=fmt)

    style_content = style_file.read_text()
    with pytest.raises(exc) if exc is not None else suppress():
        Style.from_string(style_content, format=fmt)
