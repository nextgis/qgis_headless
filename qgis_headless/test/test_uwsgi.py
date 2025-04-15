import platform
import re
import shutil
import socket
import time
from pathlib import Path
from signal import SIGTERM
from subprocess import Popen
from tempfile import TemporaryFile

import pytest
from requests import get

from .known_issues import Issues

WSGI_APP = Path(__file__).parent / "wsgi_app.py"


@pytest.mark.skipif(platform.system() == "Windows", reason="uwsgi is not supported on Windows")
@pytest.mark.timeout(10)
@pytest.mark.parametrize(
    "mode",
    [
        "dry_run",
        pytest.param("no_request", marks=Issues.UWSGI_SEGFAULT),
        pytest.param("do_request", marks=Issues.UWSGI_SEGFAULT),
    ],
)
def test_segfault(mode):
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        addr, port = sock.getsockname()

    uwsgi_path = shutil.which("uwsgi")
    assert uwsgi_path is not None

    with TemporaryFile(mode="w+") as fd:
        proc = Popen(
            [
                uwsgi_path,
                "--master",
                "--lazy-apps",
                "--enable-threads",
                "--need-app",
                "--die-on-term",
                f"--http={addr}:{port}",
                f"--wsgi-file={WSGI_APP}",
            ],
            env={"DRY_RUN": str(mode == "dry_run")},
            stderr=fd,
        )

        # Wait for uWSGI initialization
        time.sleep(5)

        if mode in ("dry_run", "do_request"):
            resp = get(f"http://{addr}:{port}")
            resp.raise_for_status()

        proc.send_signal(SIGTERM)
        proc.wait()
        assert proc.returncode == 0

        fd.seek(0)
        stderr = fd.read()
        if m := re.search(
            r"\*+ backtrace of \d+ \*+\n(.*)\n\*+ end of backtrace \*+",
            stderr,
            re.MULTILINE + re.DOTALL,
        ):
            raise RuntimeError(m.group(1))
