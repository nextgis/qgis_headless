from pathlib import Path
from xml.etree import ElementTree as ET


def _gray_renderer() -> ET.Element:
    renderer = ET.Element(
        "rasterrenderer", type="singlebandgray", grayBand="1", opacity="0.75", alphaBand="-1"
    )
    ET.SubElement(renderer, "contrastEnhancement")
    return renderer


def _palette_renderer() -> ET.Element:
    renderer = ET.Element(
        "rasterrenderer", type="paletted", band="1", opacity="0.75", alphaBand="-1"
    )
    palette = ET.SubElement(renderer, "colorPalette")
    for value, color in [(0, "#0000ff"), (255, "#ff0000")]:
        ET.SubElement(
            palette, "paletteEntry", value=str(value), color=color, alpha="255", label=str(value)
        )
    return renderer


def _pseudocolor_renderer() -> ET.Element:
    renderer = ET.Element(
        "rasterrenderer",
        type="singlebandpseudocolor",
        band="1",
        opacity="0.75",
        alphaBand="-1",
        classificationMin="0",
        classificationMax="255",
    )
    shader = ET.SubElement(renderer, "rastershader")
    ramp = ET.SubElement(
        shader,
        "colorrampshader",
        colorRampType="INTERPOLATED",
        classificationMode="1",
        minimumValue="0",
        maximumValue="255",
        clip="0",
    )
    for value, color in [(0, "#0000ff"), (255, "#ff0000")]:
        ET.SubElement(ramp, "item", value=str(value), color=color, alpha="255", label=str(value))
    return renderer


def _set_contrast_algorithm(renderer: ET.Element, algorithm: str) -> None:
    for element in renderer:
        if not element.tag.lower().endswith("contrastenhancement"):
            continue
        element.clear()
        ET.SubElement(element, "minValue").text = "0"
        ET.SubElement(element, "maxValue").text = "255"
        ET.SubElement(element, "algorithm").text = algorithm


def raster_style_xml(
    datadir: Path, renderer_type: str, algorithm: str = "StretchToMinimumMaximum"
) -> ET.Element:
    root = ET.fromstring((datadir / "raster/rounds_334.qml").read_text())
    pipe = root.find("pipe")
    assert pipe is not None
    renderer = pipe.find("rasterrenderer")
    assert renderer is not None
    if renderer_type != "multibandcolor":
        builders = {
            "singlebandgray": _gray_renderer,
            "paletted": _palette_renderer,
            "singlebandpseudocolor": _pseudocolor_renderer,
        }
        renderer_index = list(pipe).index(renderer)
        pipe.remove(renderer)
        renderer = builders[renderer_type]()
        pipe.insert(renderer_index, renderer)
    _set_contrast_algorithm(renderer, algorithm)
    return root
