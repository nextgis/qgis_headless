import os
import shutil
import signal
import socket
import time
from pathlib import Path
from subprocess import Popen

import pytest
from requests import get

WSGI_APP = Path(__file__).parent / "wsgi_app.py"

pytestmark = pytest.mark.skipif(os.name == "nt", reason="uWSGI requires POSIX")


@pytest.mark.timeout(30)
@pytest.mark.parametrize(
    "mode",
    ["dry_run", "no_request", "do_request", "late_init", "explicit_deinit", "retain_objects"],
)
def test_segfault(mode, tmp_path):
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        addr, port = sock.getsockname()

    uwsgi_path = shutil.which("uwsgi")
    assert uwsgi_path is not None

    # Preserve the selected Python build, without inheriting service options.
    environment = {key: value for key, value in os.environ.items() if not key.startswith("UWSGI_")}
    environment.update(
        DRY_RUN=str(mode == "dry_run"),
        LATE_INIT=str(mode == "late_init"),
        EXPLICIT_DEINIT=str(mode == "explicit_deinit"),
        RETAIN_OBJECTS=str(mode == "retain_objects"),
        QGIS_TEST_PROFILE_ROOT=str(tmp_path),
    )
    logfile = tmp_path / "uwsgi.log"
    with logfile.open("w+") as output:
        proc = Popen(
            [
                uwsgi_path,
                "--master",
                "--lazy-apps",
                "--processes=2",
                "--enable-threads",
                "--need-app",
                "--die-on-term",
                f"--http-socket={addr}:{port}",
                f"--wsgi-file={WSGI_APP}",
            ],
            env=environment,
            stdout=output,
            stderr=output,
            start_new_session=True,
        )
        try:
            deadline = time.monotonic() + 15
            while True:
                stderr = logfile.read_text()
                if stderr.count("WSGI app 0 (mountpoint='') ready") == 2:
                    break
                assert proc.poll() is None, stderr
                assert time.monotonic() < deadline, stderr
                time.sleep(0.05)

            if mode != "no_request":
                resp = get(f"http://{addr}:{port}", timeout=5)
                resp.raise_for_status()

            proc.send_signal(signal.SIGTERM)
            proc.wait(timeout=10)
        finally:
            # Also reap workers if startup, a request, or shutdown fails.
            try:
                os.killpg(proc.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            proc.wait(timeout=5)

    stderr = logfile.read_text()
    assert proc.returncode == 0, stderr
    # The master can exit successfully even when both workers crash.
    for failure in (
        "Segmentation Fault",
        "backtrace",
        "corrupted",
        "bad_alloc",
        "Aborted",
        "QThreadStorage:",
        "QThread: Destroyed",
        "Timers cannot",
    ):
        assert failure not in stderr, stderr
    if mode == "dry_run":
        assert stderr.count("[qgis] deinit: skipping") == 2, stderr
    else:
        expected = 3 if mode == "explicit_deinit" else 2
        assert stderr.count("[qgis] deinit: done") == expected, stderr
