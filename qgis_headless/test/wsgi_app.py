import atexit
from os import environ
from pathlib import Path

from uwsgi import log

DRY_RUN = environ.get("DRY_RUN", "").upper() in ("1", "YES", "TRUE")

if not DRY_RUN:
    import qgis_headless

data_dir = Path(__file__).parent / "data"
layer = (data_dir / "contour/data.geojson").read_text(encoding="utf-8")
style = (data_dir / "contour/rgb.qml").read_text(encoding="utf-8")
extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

if not DRY_RUN:
    log("[qgis] init: begin")
    qgis_headless.init([])
    log("[qgis] init: done")
else:
    log("[qgis] init: skipping")


@atexit.register
def deinit():
    if not DRY_RUN:
        log("[qgis] deinit: begin")
        qgis_headless.deinit()
        log("[qgis] deinit: done")
    else:
        log("[qgis] deinit: skipping")


def application(env, start_response):
    if not DRY_RUN:
        log("[qgis] render: begin")
        from qgis_headless.util import render_vector

        render_vector(layer, style, extent)
        log("[qgis] render: done")
    else:
        log("[qgis] render: skipping")

    start_response("200 OK", [("Content-Type", "text/plain")])
    return [b""]
