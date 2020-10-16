import ctypes
import json
from io import BytesIO

import pytest
from PIL import Image

from qgis_headless import MapRequest, CRS, Layer, Style, set_svg_paths
from qgis_headless.util import image_stat, render_vector, EXTENT_ONE


def test_contour(shared_datadir, reset_svg_paths):
    data = shared_datadir / 'contour.geojson'
    style = (shared_datadir / 'contour-rgb.qml').read_text()

    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    img = render_vector(data, style, extent, 1024)
    # img.save('test_contour.png')

    stat = image_stat(img)

    assert stat.alpha.min == 0, "There are no transparent pixels found"
    assert stat.red.max == 255, "Labels aren't visible"
    assert stat.green.max == 255, "Primary lines aren't visible"
    assert stat.blue.max == 255, "Primary lines aren't visible"

    assert 3 < stat.red.mean < 4
    assert 12 < stat.green.mean < 13
    assert 29 < stat.blue.mean < 30


def test_legend(shared_datadir, reset_svg_paths):
    data = shared_datadir / 'contour.geojson'
    style = (shared_datadir / 'contour-rgb.qml').read_text()

    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(CRS.from_epsg(3857))

    req.add_layer(
        Layer.from_ogr(str(data)),
        Style.from_string(style),
        label="Contour")

    img = Image.open(BytesIO(req.render_legend().to_bytes()))
    # img.save('test_legend.png')

    assert img.size == (223, 92), "Expected size is 223 x 92"

    stat = image_stat(img)
    assert stat.green.max == 255, "Primary lines aren't visible"
    assert stat.blue.max == 255, "Primary lines aren't visible"
    assert stat.red.mean == 0
    assert 1 < stat.green.mean < 3
    assert 1 < stat.blue.mean < 3

    req.set_dpi(2 * 96)
    hdpi_img = Image.open(BytesIO(req.render_legend().to_bytes()))

    assert img.size[0] < hdpi_img.size[0] and img.size[1] < hdpi_img.size[1], \
        "Higher DPI should produce bigger legend"


def test_marker_simple(shared_datadir, reset_svg_paths):
    data = shared_datadir / 'zero.geojson'
    style = (shared_datadir / 'zero-marker.qml').read_text()

    set_svg_paths([str(shared_datadir / 'marker-blue'), ])
    img = render_vector(data, style, EXTENT_ONE, 256)
    # img.save('test_marker_simple.png')

    stat = image_stat(img)

    assert stat.red.max == stat.green.max == 0, "Unexpected data in red or green channel"
    assert stat.blue.max == 255, "Blue marker is missing"


def test_marker_change(shared_datadir, reset_svg_paths):
    data = shared_datadir / 'zero.geojson'
    style = (shared_datadir / 'zero-marker.qml').read_text()

    # Render with blue marker in SVG paths
    set_svg_paths([str(shared_datadir / 'marker-blue'), ])
    img = render_vector(data, style, EXTENT_ONE, 256)
    assert image_stat(img).blue.max == 255, "Blue marker is missing"

    # Render with green marker in SVG paths
    set_svg_paths([str(shared_datadir / 'marker-green'), ])
    img = render_vector(data, style, EXTENT_ONE, 256)
    assert image_stat(img).green.max == 255, "Green marker is missing"


def test_svg_builtin(shared_datadir, reset_svg_paths):
    data = shared_datadir / 'zero.geojson'
    style = (shared_datadir / 'zero-builtin.qml').read_text()

    set_svg_paths(['/usr/share/qgis/svg', ])

    layer = Layer.from_ogr(str(data))
    style = Style.from_string(style)

    img = render_vector(layer, style, EXTENT_ONE, 256)
    assert image_stat(img).red.max == 255, "Red marker is missing"


def test_svg_resolver(shared_datadir, reset_svg_paths):
    data = shared_datadir / 'zero.geojson'
    style = (shared_datadir / 'zero-marker.qml').read_text()

    color = None

    def _resolver(source):
        target = str((shared_datadir / 'marker-{}'.format(color) / source).resolve())
        return target

    color = 'blue'
    img = render_vector(
        data, style, EXTENT_ONE, 256,
        svg_resolver=_resolver)
    assert image_stat(img).blue.max == 255, "Blue marker is missing"

    color = 'green'
    img = render_vector(
        data, style, EXTENT_ONE, 256,
        svg_resolver=_resolver)
    assert image_stat(img).green.max == 255, "Green marker is missing"

    color = 'missing'
    img = render_vector(
        data, style, EXTENT_ONE, 256,
        svg_resolver=_resolver)

    stat = image_stat(img)
    assert stat.red.min == stat.green.min == stat.blue.min == 0, "Black question mark is missing"


@pytest.mark.parametrize('mode', (
    pytest.param('resolver'),
    pytest.param('paths', marks=pytest.mark.xfail),
))
def test_svg_cache(mode, shared_datadir, reset_svg_paths):
    data = shared_datadir / 'zero.geojson'
    style = (shared_datadir / 'zero-marker.qml').read_text()
    marker = (shared_datadir / 'marker-blue' / 'marker.svg').resolve()

    svg_resolver = None
    if mode == 'resolver':
        svg_resolver = lambda _: str(marker)
    elif mode == 'paths':
        set_svg_paths([str(marker.parent), ])
    else:
        raise ValueError("Invalid mode")

    layer = Layer.from_ogr(str(data))
    style = Style.from_string(style, svg_resolver=svg_resolver)

    req = MapRequest()
    req.set_crs(CRS.from_epsg(3857))
    req.add_layer(layer, style)

    img = Image.open(BytesIO(req.render_image(EXTENT_ONE, (256, 256)).to_bytes()))
    assert image_stat(img).blue.max == 255, "Blue marker is missing"

    marker.unlink()  # Remove marker file from directory

    # And render again
    img = Image.open(BytesIO(req.render_image(EXTENT_ONE, (256, 256)).to_bytes()))
    assert image_stat(img).blue.max == 255, "Marker is missing in same MapRequest"

    # Recreata MapRequest with same Layer and Style
    req = MapRequest()
    req.set_crs(CRS.from_epsg(3857))
    req.add_layer(layer, style)

    # And render again
    img = Image.open(BytesIO(req.render_image(EXTENT_ONE, (256, 256)).to_bytes()))
    assert image_stat(img).blue.max == 255, "Marker is missing in new MapRequest"
