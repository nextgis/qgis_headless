import re
import socket
import sys
import time
from pathlib import Path
from signal import SIGTERM
from subprocess import Popen
from tempfile import TemporaryFile

import pytest
from requests import get

WSGI_APP = Path(__file__).parent / "wsgi_app.py"


@pytest.mark.timeout(10)
@pytest.mark.parametrize("mode", ["dry_run", "no_request", "do_request"])
def test_segfault(mode):
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        addr, port = sock.getsockname()

    with TemporaryFile(mode="w+") as fd:
        proc = Popen(
            [
                Path(sys.executable).parent / "uwsgi",
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
