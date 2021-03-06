import pytest
from packaging import version

from qgis_headless import Style, get_qgis_version


_version = version.parse(get_qgis_version().split('-')[0])
_version_312 = _version >= version.parse('3.12')


@pytest.mark.parametrize('file, expected', (
    pytest.param('contour-simple.qml', (), id='contour-simple'),
    pytest.param('contour-rgb.qml', ('level', ), id='contour-rgb'),
    pytest.param('contour-rbl.qml', ('level', ) if _version_312 else None, id='contour-rbl'),
    pytest.param('attributes/osm-highway.qml', ('HIGHWAY', 'NAME_EN', 'NAME'), id='osm-highway'),
    pytest.param('attributes/data-defined.qml', ('size', ), id='data-defined'),
    pytest.param('attributes/rule-based-labeling.qml', ('a', 'b', 'c') if _version_312 else None, id='rule-based-labeling'),
))
def test_sample(file, expected, shared_datadir):
    style = Style.from_file(str(shared_datadir / file))
    assert style.used_attributes() == (set(expected) if expected is not None else None)