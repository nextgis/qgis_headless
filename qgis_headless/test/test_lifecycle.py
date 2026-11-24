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
