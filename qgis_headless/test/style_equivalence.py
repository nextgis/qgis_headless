import gc
import hashlib
import json
import sys
from itertools import product
from pathlib import Path
from xml.etree import ElementTree as ET

import qgis_headless as headless
from qgis_headless.util import render_raster

from .raster_styles import large_palette_style_xml, raster_style_xml


def render_style_case(
    data: Path, renderer: str, algorithm: str, with_schema: bool
) -> dict[str, str]:
    root = (
        large_palette_style_xml(data)
        if renderer == "paletted-large"
        else raster_style_xml(data, renderer, algorithm)
    )
    schema = None
    if with_schema:
        schema = headless.LayerSchema.raster(
            headless.CRS.from_epsg(3857), ["Byte"] * 3, 5077, 4822
        )
    # The baseline binary predates LayerSchema and its keyword argument.
    kwargs = {"layer_schema": schema} if with_schema else {}
    style = headless.Style.from_string(ET.tostring(root, encoding="unicode"), **kwargs)
    filenames = ["cog-rgb-byte.tif", "cog-rgb-uint16.tif"]
    if renderer != "multibandcolor":
        filenames.append("cog-gray-float32.tif")
    layers = {name: headless.Layer.from_gdal(data / "raster" / name) for name in filenames}
    results = {}
    for repeat, filename in product(range(2), filenames):
        image = render_raster(layers[filename], style, (251440, 5977974, 1978853, 7505647))
        assert image.getchannel("A").getbbox() is not None
        key = f"{renderer}/{algorithm}/{filename}/{repeat}"
        results[key] = hashlib.sha256(image.tobytes()).hexdigest()
    return results


def render_cases(with_schema: bool) -> dict[str, str]:
    data = Path(__file__).parent / "data"
    renderers = ("multibandcolor", "singlebandgray", "singlebandpseudocolor", "paletted")
    algorithms = (
        "NoEnhancement",
        "StretchToMinimumMaximum",
        "StretchAndClipToMinimumMaximum",
        "ClipToMinimumMaximum",
    )
    results = {}
    for renderer, algorithm in product(renderers, algorithms):
        results.update(render_style_case(data, renderer, algorithm, with_schema))
    results.update(render_style_case(data, "paletted-large", "NoEnhancement", with_schema))
    return results


if __name__ == "__main__":
    headless.init([])
    try:
        result = render_cases("--schema" in sys.argv[2:])
        Path(sys.argv[1]).write_text(json.dumps(result, indent=2) + "\n")
    finally:
        gc.collect()
        headless.deinit()
