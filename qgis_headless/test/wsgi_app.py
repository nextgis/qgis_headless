import atexit
from os import environ, getpid
from pathlib import Path

from uwsgi import log

DRY_RUN = environ.get("DRY_RUN", "").upper() in ("1", "YES", "TRUE")
LATE_INIT = environ.get("LATE_INIT", "").upper() in ("1", "YES", "TRUE")
EXPLICIT_DEINIT = environ.get("EXPLICIT_DEINIT", "").upper() in ("1", "YES", "TRUE")
RETAIN_OBJECTS = environ.get("RETAIN_OBJECTS", "").upper() in ("1", "YES", "TRUE")
retained_objects = []

# Each worker must have its own profile and must not access the user's keyring.
profile = Path(environ["QGIS_TEST_PROFILE_ROOT"]) / str(getpid())
for directory in ("home", "config/QGIS", "auth", "runtime"):
    (profile / directory).mkdir(parents=True, exist_ok=True)
(profile / "runtime").chmod(0o700)
(profile / "config/QGIS/QGIS3.ini").write_text("[auth]\nuse_password_helper=false\n")
environ.update(
    HOME=str(profile / "home"),
    XDG_CONFIG_HOME=str(profile / "config"),
    XDG_RUNTIME_DIR=str(profile / "runtime"),
    QGIS_CUSTOM_CONFIG_PATH=str(profile / "config"),
    QGIS_AUTH_DB_DIR_PATH=str(profile / "auth"),
    QGIS_CONTINUOUS_INTEGRATION_RUN="true",
    DBUS_SESSION_BUS_ADDRESS=f"unix:path={profile / 'no-session-bus'}",
)

if not DRY_RUN:
    import qgis_headless

data_dir = Path(__file__).parent / "data"
layer = (data_dir / "contour/data.geojson").read_text(encoding="utf-8")
style = (data_dir / "contour/rgb.qml").read_text(encoding="utf-8")
extent = (9757454.0, 6450871.0, 9775498.0, 6465163.0)

if not DRY_RUN and not LATE_INIT:
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
        if LATE_INIT:
            qgis_headless.init([])
        log("[qgis] render: begin")
        from qgis_headless.util import render_vector

        if RETAIN_OBJECTS:
            native_layer = qgis_headless.Layer.from_ogr(layer)
            native_style = qgis_headless.Style.from_string(style)
            image = render_vector(native_layer, native_style, extent)
            retained_objects.extend((native_layer, native_style, image))
            retained_objects.append(retained_objects)
        else:
            render_vector(layer, style, extent)
        log("[qgis] render: done")
        if EXPLICIT_DEINIT:
            deinit()
    else:
        log("[qgis] render: skipping")

    start_response("200 OK", [("Content-Type", "text/plain")])
    return [b""]
