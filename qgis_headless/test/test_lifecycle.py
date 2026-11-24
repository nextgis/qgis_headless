import os
import subprocess
import sys
from textwrap import dedent

import pytest


@pytest.mark.parametrize("cleanup", ["explicit", "atexit"])
def test_deinit(cleanup, tmp_path):
    for directory in ("home", "config/QGIS", "auth"):
        (tmp_path / directory).mkdir(parents=True, exist_ok=True)
    (tmp_path / "config/QGIS/QGIS3.ini").write_text("[auth]\nuse_password_helper=false\n")
    environment = dict(
        os.environ,
        HOME=str(tmp_path / "home"),
        XDG_CONFIG_HOME=str(tmp_path / "config"),
        QGIS_CUSTOM_CONFIG_PATH=str(tmp_path / "config"),
        QGIS_AUTH_DB_DIR_PATH=str(tmp_path / "auth"),
        QGIS_CONTINUOUS_INTEGRATION_RUN="true",
        DBUS_SESSION_BUS_ADDRESS=f"unix:path={tmp_path / 'no-session-bus'}",
        CLEANUP=cleanup,
    )
    result = subprocess.run(
        [
            sys.executable,
            "-c",
            dedent("""
                import atexit
                import os
                import qgis_headless as qh

                def cleanup():
                    qh.deinit()
                    qh.deinit()
                    print("Cleanup completed", flush=True)

                qh.deinit()
                qh.init([])
                if os.environ["CLEANUP"] == "atexit":
                    atexit.register(cleanup)
                else:
                    cleanup()
            """),
        ],
        env=environment,
        capture_output=True,
        text=True,
        timeout=15,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    assert result.stdout.count("Cleanup completed") == 1, result.stdout + result.stderr
    assert "Exception ignored in atexit callback" not in result.stderr, result.stderr


@pytest.mark.parametrize(
    "resource", ["vector", "memory", "raster", "style", "request", "project", "crs"]
)
@pytest.mark.parametrize("release", ["gc", "traceback", "interpreter"])
def test_resources_outlive_deinit(resource, release, tmp_path):
    for directory in ("home", "config/QGIS", "auth"):
        (tmp_path / directory).mkdir(parents=True, exist_ok=True)
    (tmp_path / "config/QGIS/QGIS3.ini").write_text("[auth]\nuse_password_helper=false\n")
    environment = dict(
        os.environ,
        PYTHONDEVMODE="1",
        HOME=str(tmp_path / "home"),
        XDG_CONFIG_HOME=str(tmp_path / "config"),
        QGIS_CUSTOM_CONFIG_PATH=str(tmp_path / "config"),
        QGIS_AUTH_DB_DIR_PATH=str(tmp_path / "auth"),
        QGIS_CONTINUOUS_INTEGRATION_RUN="true",
        DBUS_SESSION_BUS_ADDRESS=f"unix:path={tmp_path / 'no-session-bus'}",
        RESOURCE=resource,
        RELEASE=release,
    )
    result = subprocess.run(
        [
            sys.executable,
            "-c",
            dedent("""
                import gc
                import os
                from pathlib import Path
                import qgis_headless as qh
                from qgis_headless.util import EXTENT_ONE, render_vector

                data = Path(qh.__file__).parent / "test/data"
                qh.init([])

                def create():
                    kind = os.environ["RESOURCE"]
                    if kind == "vector":
                        return qh.Layer.from_ogr(data / "zero/data.geojson")
                    if kind == "memory":
                        return qh.Layer.from_data(qh.Layer.GT_POINT, qh.CRS.from_epsg(3857), (), ())
                    if kind == "raster":
                        return qh.Layer.from_gdal(data / "raster/rounds.tif")
                    if kind == "project":
                        return qh.Project.from_file(data / "projects/magadan/project.qgz")
                    if kind == "crs":
                        return qh.CRS.from_epsg(3857)
                    layer = qh.Layer.from_ogr(data / "zero/data.geojson")
                    style = qh.Style.from_file(data / "zero/label/variables.qml")
                    image = render_vector(layer, style, EXTENT_ONE)
                    if kind == "style":
                        return style
                    request = qh.MapRequest()
                    request.set_crs(qh.CRS.from_epsg(3857))
                    request.add_layer(layer, style)
                    return request

                def fail():
                    resource = create()
                    raise AssertionError("Retain resource in traceback")

                if os.environ["RELEASE"] == "traceback":
                    try:
                        fail()
                    except AssertionError as error:
                        retained = error
                else:
                    retained = [create()]
                    retained.append(retained)
                qh.deinit()
                qh.deinit()
                print("Deinit completed", flush=True)
                if os.environ["RELEASE"] != "interpreter":
                    del retained
                    gc.collect()
                    print("Resources released", flush=True)
            """),
        ],
        env=environment,
        capture_output=True,
        text=True,
        timeout=15,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    assert "Deinit completed" in result.stdout, result.stdout + result.stderr
    if release != "interpreter":
        assert "Resources released" in result.stdout, result.stdout + result.stderr
