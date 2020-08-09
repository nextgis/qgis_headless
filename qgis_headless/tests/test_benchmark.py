import pytest
from qgis_headless import MapRequest, CRS, Layer, Style


SIZES = (256, 362, 512, 724, 1024)


@pytest.mark.benchmark(group='empty')
@pytest.mark.parametrize('size', SIZES)
def test_empty(size, benchmark):
    mreq = MapRequest()
    mreq.set_dpi(96)
    mreq.set_crs(CRS.from_epsg(CRS.EPSG_3857))

    def _render_image():
        mreq.render_image((-1, -1, 1, 1), (size, size))     

    benchmark(_render_image)


@pytest.mark.benchmark(group='contour')
@pytest.mark.parametrize('size', SIZES)
def test_contour(size, benchmark, shared_datadir):
    data = (shared_datadir / 'contour.geojson').read_text()  
    style = (shared_datadir / 'contour-simple.qml').read_text()
    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    mreq = MapRequest()
    mreq.set_dpi(96)
    mreq.set_crs(CRS.from_epsg(CRS.EPSG_3857))
    mreq.add_layer(Layer.from_ogr(str(data)), Style.from_string(style))

    def _render_image():
        mreq.render_image(extent, (size, size))

    benchmark(_render_image)
