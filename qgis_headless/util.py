from collections import namedtuple
from io import BytesIO

from qgis_headless import MapRequest, CRS, Layer, Style


BandStat = namedtuple("BandStat", ['min', 'max', 'mean'])
ImageStat = namedtuple("ImageStat", ['red', 'green', 'blue', 'alpha'])

EXTENT_ONE = (-0.5, -0.5, 0.5, 0.5)


def to_pil(source):
    from PIL import Image  # Optional dependency

    im = Image.frombuffer('RGBA', source.size(), source.to_bytes(), 'raw')

    # Keep reference to the original image, thus the source image won't be
    # destroyed before PIL image
    im._qgis_headless_source = source

    return im


def image_stat(image):
    from PIL.ImageStat import Stat  # Optional dependency

    stat = Stat(image)
    return ImageStat(*[
        BandStat(stat.extrema[b][0], stat.extrema[b][1], stat.mean[b])
        for b in range(4)
    ])


def render_vector(layer, style, extent, size=(256, 256), crs=CRS.from_epsg(3857), svg_resolver=None):
    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(crs)

    if isinstance(size, int):
        size = (size, int(size * (extent[3] - extent[1]) / (extent[2] - extent[0])))

    if not isinstance(layer, Layer):
        layer = Layer.from_ogr(str(layer))

    if not isinstance(style, Style):
        style = Style.from_string(style, svg_resolver=svg_resolver)

    req.add_layer(layer, style)

    return to_pil(req.render_image(extent, size))
