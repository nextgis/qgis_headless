from io import BytesIO
import ctypes
import pytest
from PIL import Image

from qgis_headless import MapRequest, CRS, Layer, Style
from qgis_headless.util import image_stat


EXTENT_ONE = (-0.5, -0.5, 0.5, 0.5)


def render_vector(data, qml, extent, size, svg_paths=None):
    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(CRS.from_epsg(3857))
    if svg_paths is not None:
        req.set_svg_paths(svg_paths)

    if isinstance(size, int):
        size = (size, int(size * (extent[3] - extent[1]) / (extent[2] - extent[0])))

    req.add_layer(
        Layer.from_ogr(str(data)),
        Style.from_string(qml))

    image = req.render_image(extent, size)
    return Image.open(BytesIO(image.to_bytes()))


def test_contour(fetch, shared_datadir):
    data = (shared_datadir / 'contour.geojson').read_text()
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


def test_legend(fetch, shared_datadir):
    data = (shared_datadir / 'contour.geojson').read_text()
    style = (shared_datadir / 'contour-rgb.qml').read_text()

    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    base = 1024
    size = (base, int(base * (extent[3] - extent[1]) / (extent[2] - extent[0])))

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


def test_marker_simple(fetch, shared_datadir):
    data = (shared_datadir / 'zero.geojson').read_text()
    style = (shared_datadir / 'zero-marker.qml').read_text()

    img = render_vector(data, style, EXTENT_ONE, 256, svg_paths=[
        str(shared_datadir / 'marker-blue'), ])
    # img.save('test_marker_simple.png')

    stat = image_stat(img)

    assert stat.red.max == stat.green.max == 0, "Unexpected data in red or green channel"
    assert stat.blue.max == 255, "Blue marker is missing"


def test_marker_change(fetch, shared_datadir):
    data = (shared_datadir / 'zero.geojson').read_text()
    style = (shared_datadir / 'zero-marker.qml').read_text()

    # Render with blue marker in SVG paths
    img = render_vector(data, style, EXTENT_ONE, 256, svg_paths=[
        str(shared_datadir / 'marker-blue'), ])
    assert image_stat(img).blue.max == 255, "Blue marker is missing"

    # Render with green marker in SVG paths
    img = render_vector(data, style, EXTENT_ONE, 256, svg_paths=[
        str(shared_datadir / 'marker-green'), ])
    assert image_stat(img).green.max == 255, "Green marker is missing"


@pytest.mark.xfail(reason="SVG marker cache doesn't work yet!")
def test_marker_cache(fetch, shared_datadir):
    data = (shared_datadir / 'zero.geojson').read_text()
    style = (shared_datadir / 'zero-marker.qml').read_text()

    img = render_vector(data, style, EXTENT_ONE, 256, svg_paths=[
        str(shared_datadir / 'marker-blue'), ])
    assert image_stat(img).blue.max == 255, "Blue marker is missing"

    (shared_datadir / 'marker-blue' / 'marker.svg').unlink()

    # Render with green marker in SVG paths
    img = render_vector(data, style, EXTENT_ONE, 256, svg_paths=[
        str(shared_datadir / 'marker-blue'), ])
    assert image_stat(img).blue.max == 255, "Blue marker is missing"