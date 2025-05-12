from qgis_headless import Layer
from qgis_headless.util import render_legend


def check_chart(image):
    CHART_LEGEND_OFFSET_X = 12
    CHART_LEGEND_OFFSET_Y = 45
    CHART_LEGEND_SIZE = (15, 15)
    CHART_LEGEND_COLORS = [(23, 111, 193, 255), (51, 180, 100, 255), (180, 51, 62, 255)]
    CHART_LEGEND_INTERVAL_Y = 10

    for i in range(3):
        legend_pixel_y = (
            CHART_LEGEND_OFFSET_Y + (CHART_LEGEND_SIZE[1] + CHART_LEGEND_INTERVAL_Y) * i
        )
        assert image.getpixel((CHART_LEGEND_OFFSET_X, legend_pixel_y)) == CHART_LEGEND_COLORS[i]


def test_pie_chart(save_img, shared_datadir):
    layer = Layer.from_ogr(shared_datadir / "charts" / "districts.geojson")
    style = (shared_datadir / "charts" / "pie-chart.qml").read_text()
    check_chart(save_img(render_legend(layer, style)))


def test_bar_chart(save_img, shared_datadir):
    layer = Layer.from_ogr(shared_datadir / "charts" / "districts.geojson")
    style = (shared_datadir / "charts" / "bar-chart.qml").read_text()
    check_chart(save_img(render_legend(layer, style)))
