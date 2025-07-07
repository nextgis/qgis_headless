import gc
import json
import re
from pathlib import Path
from typing import Optional

import _qgis_headless
import pytest
import requests


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
def fetch(cache):
    cache_path = cache.makedir("fetch")

    def do(url, mode="binary"):
        name = re.sub(r"^http[s]?\:\/\/", "", url.lower())
        name = re.sub(r"[^A-z0-9\_]", "_", name)
        name = re.sub(r"\_{2,}", "_", name)

        fn = cache_path / name
        if fn.exists():
            if mode == "file":
                return fn
            with fn.open("rb") as fd:
                data = fd.read()
        else:
            resp = requests.get(url)
            resp.raise_for_status()
            data = resp.content
            with fn.open("wb") as fd:
                fd.write(data)

        if mode == "binary":
            return data
        elif mode == "text":
            return data.decode("utf-8")
        elif mode == "json":
            return json.loads(data)
        elif mode == "file":
            return fn

    return do


@pytest.fixture()
def save_img(request):
    def _do(img, name: Optional[str] = None, *, suffix: Optional[str] = None):
        return img

    if img_output_path := request.config.getoption("--img-output"):
        base = Path(img_output_path) / str(request.function.__name__)
        subtest_name = request.node.callspec.id if hasattr(request.node, "callspec") else None
        if subtest_name:
            base = base / str(subtest_name)

        def _do(img, name: Optional[str] = None, *, suffix: Optional[str] = None):
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
