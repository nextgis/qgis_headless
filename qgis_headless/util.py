from binascii import a2b_hex
from collections import namedtuple

from qgis_headless import MapRequest, CRS, Layer, Style


BandStat = namedtuple("BandStat", ['min', 'max', 'mean'])
ImageStat = namedtuple("ImageStat", ['red', 'green', 'blue', 'alpha'])

EXTENT_ONE = (-0.5, -0.5, 0.5, 0.5)

# Sample WKB geometries

WKB_POINT_00 = a2b_hex('010100000000000000000000000000000000000000')  # POINT(0 0)
WKB_POINT_11 = a2b_hex('0101000000000000000000f03f000000000000f03f')  # POINT(1 1)

WKB_POINTZ_000 = a2b_hex('0101000080000000000000000000000000000000000000000000000000')  # POINT Z(0 0 0)
WKB_POINTZ_111 = a2b_hex('0101000080000000000000f03f000000000000f03f000000000000f03f')  # POINT Z(1 1 1)

WKB_LINESTRING = a2b_hex('01020000000200000000000000000000000000000000000000000000000000f03f000000000000f03f')  # LINESTRING(0 0, 1 1)


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

def cmp_colors(a, b):
    return float(sum((ca - cb)**2 for ca, cb in zip(a, b)))


def render_vector(layer, style, extent, size=(256, 256), crs=CRS.from_epsg(3857), svg_resolver=None, dpi=96):
    req = MapRequest()
    req.set_dpi(dpi)
    req.set_crs(crs)

    if isinstance(size, int):
        size = (size, int(size * (extent[3] - extent[1]) / (extent[2] - extent[0])))

    if not isinstance(layer, Layer):
        layer = Layer.from_ogr(str(layer))

    if not isinstance(style, Style):
        style = Style.from_string(style, svg_resolver=svg_resolver)
    else:
        assert svg_resolver is None, f"ignoring svg_resolver: {svg_resolver!r}"


    req.add_layer(layer, style)

    return to_pil(req.render_image(extent, size))


def render_raster(layer, style, extent, size=(256, 256), crs=CRS.from_epsg(3857)):
    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(crs)

    if isinstance(size, int):
        size = (size, int(size * (extent[3] - extent[1]) / (extent[2] - extent[0])))

    if not isinstance(layer, Layer):
        layer = Layer.from_gdal(str(layer))

    if not isinstance(style, Style):
        style = Style.from_string(style)

    req.add_layer(layer, style)

    return to_pil(req.render_image(extent, size))
