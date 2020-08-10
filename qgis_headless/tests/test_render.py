from io import BytesIO
import ctypes
import pytest
from PIL import Image

from qgis_headless import MapRequest, CRS, Layer, Style
from qgis_headless.util import image_stat


def render_vector(data, qml, extent, size):
    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(CRS.from_epsg(3857))

    req.add_layer(
        Layer.from_ogr(str(data)),
        Style.from_string(qml))

    image = req.render_image(extent, size)
    return Image.open(BytesIO(image.to_bytes()))


def test_contour(fetch, shared_datadir):
    data = (shared_datadir / 'contour.geojson').read_text()
    style = (shared_datadir / 'contour-rgb.qml').read_text()

    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    base = 1024
    size = (base, int(base * (extent[3] - extent[1]) / (extent[2] - extent[0])))

    img = render_vector(data, style, extent, size)
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
        Style.from_string(style))

    img = Image.open(BytesIO(req.render_legend((240, 120)).to_bytes()))
    # img.save('test_legend.png')

    stat = image_stat(img)

    assert 240 < stat.red.mean < 245
    assert 240 < stat.green.mean < 245
    assert 240 < stat.blue.mean < 245

