import gc
import subprocess
import sys
from pathlib import Path

import pytest

import _qgis_headless


@pytest.fixture
def svg_http_url(tmp_path):
    (tmp_path / "marker.svg").write_text(
        '<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32">'
        '<rect width="32" height="32" fill="#ff0000"/></svg>'
    )
    # A separate interpreter can answer while the renderer holds Python's GIL.
    server_script = Path(__file__).with_name("svg_http_server.py")
    with subprocess.Popen(
        [sys.executable, "-u", str(server_script)],
        cwd=tmp_path,
        stdout=subprocess.PIPE,
        text=True,
    ) as process:
        try:
            port = int(process.stdout.readline())
            yield f"http://127.0.0.1:{port}/marker.svg"
        finally:
            process.terminate()
            process.wait(timeout=5)


def pytest_addoption(parser):
    parser.addoption(
        "--img-output",
        type=str,
        default=None,
        help="dump output images into this directory",
    )


@pytest.fixture(scope="session", autouse=True)
def setup():
    _qgis_headless.init([])

    yield

    # Force garbage collection before qgis deinitialization. In case of
    # destroying Layers and Styles after deinit() will cause segmentation fault.
    gc.collect()

    _qgis_headless.deinit()


@pytest.fixture()
def reset_svg_paths():
    _qgis_headless.set_svg_paths([])


@pytest.fixture()
def save_img(request):
    def _do(img, name: str | None = None, *, suffix: str | None = None):
        return img

    if img_output_path := request.config.getoption("--img-output"):
        base = Path(img_output_path) / str(request.function.__name__)
        subtest_name = request.node.callspec.id if hasattr(request.node, "callspec") else None
        if subtest_name:
            base = base / str(subtest_name)

        def _do(img, name: str | None = None, *, suffix: str | None = None):
            if name:
                directory = base
            else:
                directory = base.parent
                name = base.name

            if name.endswith(".png"):
                name = name[:-4]

            if suffix:
                name += suffix

            name += ".png"

            directory.mkdir(parents=True, exist_ok=True)

            img.save(directory / name)
            return img

    return _do
