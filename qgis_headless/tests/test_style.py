import pytest
from packaging import version

from qgis_headless import Style, StyleValidationError, get_qgis_version


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
))
def test_attributes(file, expected, shared_datadir):
    style = Style.from_file(str(shared_datadir / file))
    assert style.used_attributes() == (set(expected) if expected is not None else None)