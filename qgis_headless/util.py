from binascii import a2b_hex
from collections import namedtuple
from pathlib import Path
from typing import TYPE_CHECKING, Callable, Optional, Tuple, Union

from qgis_headless import CRS, Image, Layer, MapRequest, Style, StyleFormat

if TYPE_CHECKING:
    import PIL.Image

    PilImage = PIL.Image.Image


BandStat = namedtuple("BandStat", ["min", "max", "mean", "nonzero"])
ImageStat = namedtuple("ImageStat", ["red", "green", "blue", "alpha"])

EXTENT_ONE = (-0.5, -0.5, 0.5, 0.5)

# fmt: off
WKB_POINT_00 = a2b_hex("010100000000000000000000000000000000000000")  # POINT(0 0)
WKB_POINT_11 = a2b_hex("0101000000000000000000f03f000000000000f03f")  # POINT(1 1)
WKB_POINTZ_000 = a2b_hex("0101000080000000000000000000000000000000000000000000000000")  # POINT Z(0 0 0)
WKB_POINTZ_111 = a2b_hex("0101000080000000000000f03f000000000000f03f000000000000f03f")  # POINT Z(1 1 1)
WKB_LINESTRING = a2b_hex("01020000000200000000000000000000000000000000000000000000000000f03f000000000000f03f")  # LINESTRING(0 0, 1 1)
# fmt: on

RED = (255, 0, 0, 255)
GREEN = (0, 255, 0, 255)
BLUE = (0, 0, 255, 255)
TRANSPARENT = (0, 0, 0, 0)
GRAY_128 = (128, 128, 128, 128)


def to_pil(source: Image) -> "PilImage":
    from PIL import Image  # Optional dependency

    im = Image.frombuffer("RGBA", source.size(), source.to_bytes(), "raw")

    # Keep reference to the original image, thus the source image won't be
    # destroyed before PIL image
    im._qgis_headless_source = source

    return im


def image_stat(image: "PilImage") -> ImageStat:
    """
    Calculate statistical information for an image.

    This function computes statistics for each band (R, G, B, A) of the given image,
    including the minimum and maximum pixel values, the mean pixel value, and the
    count of non-zero pixels.

    :param image: A PIL Image object to analyze.
    :type image: PIL.Image.Image
    :return: An ImageStat object containing the computed statistics for each band.
    :rtype: ImageStat
    """

    from PIL.ImageStat import Stat  # Optional dependency

    stat = Stat(image)

    nonzero = [0, 0, 0, 0]
    for r, g, b, a in image.getdata():
        if r != 0:
            nonzero[0] += 1
        if g != 0:
            nonzero[1] += 1
        if b != 0:
            nonzero[2] += 1
        if a != 0:
            nonzero[3] += 1

    return ImageStat(
        *[
            BandStat(stat.extrema[band][0], stat.extrema[band][1], stat.mean[band], nonzero[band])
            for band in range(4)
        ]
    )


def is_same_color(a: Tuple[int, int, int, int], b: Tuple[int, int, int, int]) -> bool:
    color_channels_delta = sum(abs(ca - cb) for ca, cb in zip(a[:3], b[:3]))
    alpha_channel_delta = abs(a[3] - b[3]) / 255.0
    return color_channels_delta <= 3 and alpha_channel_delta < 0.05


def render_vector(
    layer: Union[Layer, Path, str],
    style: Union[Style, str],
    extent: Tuple[float, float, float, float],
    size: Union[int, Tuple[int, int]] = (256, 256),
    dpi: int = 96,
    crs: CRS = CRS.from_epsg(3857),
    svg_resolver: Optional[Callable[[str], str]] = None,
    style_format: Optional[StyleFormat] = None,
):
    """Renders a vector image for a given map layer and style."""
    req = MapRequest()
    req.set_dpi(dpi)
    req.set_crs(crs)

    if isinstance(size, int):
        size = (size, int(size * (extent[3] - extent[1]) / (extent[2] - extent[0])))

    if not isinstance(layer, Layer):
        layer = Layer.from_ogr(layer)

    if not isinstance(style, Style):
        style = Style.from_string(
            style,
            svg_resolver=svg_resolver,
            format=style_format if style_format else StyleFormat.QML,
        )
    else:
        assert svg_resolver is None, f"ignoring svg_resolver: {svg_resolver!r}"
        assert style_format is None, f"ignoring style_format: {style_format!r}"

    req.add_layer(layer, style)

    return to_pil(req.render_image(extent, size))


def render_raster(
    layer: Union[Layer, Path, str],
    style: Union[Style, str],
    extent: Tuple[float, float, float, float],
    size: Union[int, Tuple[int, int]] = (256, 256),
    crs: CRS = CRS.from_epsg(3857),
):
    """Renders a raster image for a given map layer and style."""
    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(crs)

    if isinstance(size, int):
        size = (size, int(size * (extent[3] - extent[1]) / (extent[2] - extent[0])))

    if not isinstance(layer, Layer):
        layer = Layer.from_gdal(layer)

    if not isinstance(style, Style):
        style = Style.from_string(style)

    req.add_layer(layer, style)

    return to_pil(req.render_image(extent, size))


def render_legend(
    layer: Union[Layer, Path, str],
    style: Union[Style, Path, str],
    layerName: str = "",
    size: Tuple[int, int] = (256, 256),
    dpi: int = 96,
    crs: CRS = CRS.from_epsg(3857),
    svg_resolver: Optional[Callable[[str], str]] = None,
    style_format: Optional[StyleFormat] = None,
):
    """Renders a legend image for a given map layer and style."""
    request = MapRequest()
    request.set_dpi(dpi)
    request.set_crs(crs)

    if not isinstance(layer, Layer):
        layer = Layer.from_ogr(layer)

    if isinstance(style, Style):
        assert svg_resolver is None, f"ignoring svg_resolver: {svg_resolver!r}"
        assert style_format is None, f"ignoring style_format: {style_format!r}"

    elif isinstance(style, Path):
        style = Style.from_file(
            style,
            svg_resolver=svg_resolver,
            format=style_format if style_format else StyleFormat.QML,
        )
    elif isinstance(style, str):
        style = Style.from_string(
            style,
            svg_resolver=svg_resolver,
            format=style_format if style_format else StyleFormat.QML,
        )

    request.add_layer(layer, style, layerName)

    return to_pil(request.render_legend(size))
