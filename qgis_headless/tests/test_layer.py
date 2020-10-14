from binascii import a2b_hex
from datetime import date, time
from xml.sax.saxutils import quoteattr

import pytest 
from qgis_headless import Layer, CRS
from qgis_headless.util import render_vector, EXTENT_ONE, image_stat

# Sample WKB geometries

WKB_POINT_00 = a2b_hex('010100000000000000000000000000000000000000')  # POINT(0 0)
WKB_POINT_11 = a2b_hex('0101000000000000000000f03f000000000000f03f')  # POINT(1 1)

WKB_LINESTRING = a2b_hex('01020000000200000000000000000000000000000000000000000000000000f03f000000000000f03f')  # LINESTRING(0 0, 1 1)


def test_from_data_red_circle(shared_datadir, reset_svg_paths):
    style = (shared_datadir / 'zero-red-circle.qml').read_text()
    crs = CRS.from_epsg(3857)

    # layer = str(shared_datadir / 'zero.geojson')
    layer = Layer.from_data(
        Layer.GT_POINT, crs, (
            ('f_integer', Layer.FT_INTEGER),
            ('f_real', Layer.FT_REAL),
            ('f_string', Layer.FT_STRING),
        ), (
            (1, WKB_POINT_00, (1, 0.33, 'foo')),
            (2, WKB_POINT_11, (1, 0.33, None)),
        )
    )

    img = render_vector(layer, style, EXTENT_ONE, 256)
    # img.save("test_from_data_red_circle.png")

    stat = image_stat(img)
    assert stat.red.max == 255, "Red marker missing"


def test_from_data_linestring(shared_datadir, reset_svg_paths):
    crs = CRS.from_epsg(3857)
    empty_tuple = tuple()
    Layer.from_data(
        Layer.GT_LINESTRING, crs, empty_tuple, (
            (1, WKB_LINESTRING, empty_tuple),
            (2, WKB_LINESTRING, empty_tuple),
        )
    )


@pytest.mark.parametrize('ftype, fvalue, cond', (
    pytest.param(Layer.FT_INTEGER, None, "field IS NULL", id='integer null'),
    pytest.param(Layer.FT_INTEGER, 42, "field = 42", id='integer 42'),
    pytest.param(Layer.FT_REAL, 0.25, "field = 0.25", id='real'),
    pytest.param(Layer.FT_STRING, '', "field = ''", id='string empty'),
    pytest.param(Layer.FT_STRING, '0' * 512, "length(field) = 512", id='string 512'),
    pytest.param(Layer.FT_DATE, (1917, 11, 7), "field = to_date('1917-11-07')", id='date'),
    pytest.param(Layer.FT_TIME, (16, 20, 0), "field = to_time('16:20:00')", id='time'),
    pytest.param(Layer.FT_DATETIME, (1917, 11, 7, 16, 20, 0), "field = to_datetime('1917-11-07 16:20:00')", id='datetime'),
    pytest.param(Layer.FT_INTEGER64, 2**62, "field = 4611686018427387904", id='integer64 2**62'),
))
def test_field_type(ftype, fvalue, cond, shared_datadir, reset_svg_paths):
    style = (shared_datadir / 'zero-placeholder.qml').read_text()
    crs = CRS.from_epsg(3857)

    layer = Layer.from_data(
        Layer.GT_POINT, crs, (('field', ftype), ),
        ((1, WKB_POINT_00, (fvalue, )), ))
    
    style = style.replace('"/* {{{ */ FALSE /* }}} */"', quoteattr(cond))

    img = render_vector(layer, style, EXTENT_ONE, 256)
    # img.save("test_field_type.png")

    stat = image_stat(img)
    assert stat.red.max == 255, "Red marker missing"
