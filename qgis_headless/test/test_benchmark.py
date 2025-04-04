import pytest

from qgis_headless import CRS, LT_VECTOR, Layer, MapRequest, Style
from qgis_headless.util import to_pil

SIZES = (256, 362, 512, 724, 1024)


@pytest.mark.benchmark(group="empty")
@pytest.mark.parametrize("size", SIZES)
def test_empty(size, benchmark):
    mreq = MapRequest()
    mreq.set_dpi(96)
    mreq.set_crs(CRS.from_epsg(3857))

    def _render_image():
        to_pil(mreq.render_image((-1, -1, 1, 1), (size, size)))

    benchmark(_render_image)


@pytest.mark.benchmark(group="contour")
@pytest.mark.parametrize("size", SIZES)
def test_contour(size, benchmark, shared_datadir):
    data = (shared_datadir / "contour/data.geojson").read_text()
    style = (shared_datadir / "contour/simple.qml").read_text()
    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    mreq = MapRequest()
    mreq.set_dpi(96)
    mreq.set_crs(CRS.from_epsg(3857))
    mreq.add_layer(Layer.from_ogr(data), Style.from_string(style))

    def _render_image():
        to_pil(mreq.render_image(extent, (size, size)))

    benchmark(_render_image)


@pytest.mark.benchmark(group="bloat")
@pytest.mark.parametrize(
    "payload",
    ["default", "string"] + [2**i for i in (8, 10, 12, 14, 16, 18)],
)
def test_bloat(payload, benchmark):
    extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

    crs = CRS.from_epsg(3857)
    layer = Layer.from_data(Layer.GT_LINESTRING, crs, (), ())
    style = Style.from_defaults(
        layer_type=LT_VECTOR,
        layer_geometry_type=Layer.GT_LINESTRING,
        color=(255, 0, 0, 255),
    )

    if payload != "default":
        qml = style.to_string()
        if payload != "string":
            inspos = len(qml) - len("</qgis>") - 1
            qml_head = qml[:inspos]
            qml_tail = qml[inspos:]
            assert qml_tail.strip() == "</qgis>"
            baloon = "<!-- baloon -->\n"
            qml = qml_head + (baloon * payload) + qml_tail
        style = Style.from_string(qml)

    def _render_image():
        mreq = MapRequest()
        mreq.set_dpi(96)
        mreq.set_crs(crs)
        mreq.add_layer(layer, style)
        mreq.render_image(extent, (512, 512))

    benchmark(_render_image)
