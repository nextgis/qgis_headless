import contextlib

import pytest
from packaging import version

from qgis_headless import (
    LT_RASTER,
    LT_UNKNOWN,
    LT_VECTOR,
    Layer,
    Style,
    StyleValidationError,
    StyleTypeMismatch,
    get_qgis_version,
)


QGIS_VERSION = version.parse(get_qgis_version().split('-')[0])


def test_non_existent_file(shared_datadir):
    with pytest.raises(StyleValidationError):
        Style.from_file(str(shared_datadir / 'missing.qml'))


def test_invalid_file(shared_datadir):
    with pytest.raises(StyleValidationError):
        Style.from_file(str(shared_datadir / 'invalid.qml'))


def test_empty_string(shared_datadir):
    with pytest.raises(StyleValidationError):
        Style.from_string('')


@pytest.mark.parametrize('file, expected', (
    pytest.param('contour-simple.qml', (), id='contour-simple'),
    pytest.param('contour-rgb.qml', ('level', ), id='contour-rgb'),
    pytest.param('contour-rbl.qml', ('level', ) if (QGIS_VERSION >= version.parse('3.12')) else None, id='contour-rbl'),
    pytest.param('attributes/osm-highway.qml', ('HIGHWAY', 'NAME_EN', 'NAME'), id='osm-highway'),
    pytest.param('attributes/data-defined.qml', ('size', ), id='data-defined'),
    pytest.param('attributes/rule-based-labeling.qml', ('a', 'b', 'c') if (QGIS_VERSION >= version.parse('3.12')) else None, id='rule-based-labeling'),
    pytest.param('diagram/industries.qml', ('zern', 'ovosch', 'sad', 'vinograd', 'efir', 'skotovod', 'svinovod', 'ptitcevod', 'total'),
                 id='diagram', marks=pytest.mark.xfail(reason='https://github.com/qgis/QGIS/issues/33810'))
))
def test_attributes(file, expected, shared_datadir):
    style = Style.from_file(str(shared_datadir / file))
    assert style.used_attributes() == (set(expected) if expected is not None else None)


def test_attributes_default():
    style = Style.from_defaults()
    attrs = style.used_attributes()
    assert attrs == set()


@pytest.mark.parametrize('style, layer_type, exc', (
    ('point-style.qml', LT_UNKNOWN, None),
    ('point-style.qml', LT_VECTOR, None),
    ('point-style.qml', LT_RASTER, StyleTypeMismatch),
    ('raster/rounds.qml', LT_UNKNOWN, None),
    ('raster/rounds.qml', LT_VECTOR, StyleTypeMismatch),
    ('raster/rounds.qml', LT_RASTER, None),
))
def test_layer_type(style, layer_type, exc, shared_datadir):

    with pytest.raises(exc) if exc is not None else contextlib.suppress():
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
