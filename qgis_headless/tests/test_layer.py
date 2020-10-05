from binascii import a2b_hex

import pytest 
from qgis_headless import Layer, CRS


# Constant values for geometry types  are imported from GDAL/OGR

GT_POINT = 1
GT_LINESTRING = 2
GT_POLYGON = 3
GT_MULTIPOINT = 4
GT_MULTILINESTRING = 5
GT_MULTIPOLYGON = 6

# Constant values for field types are imported from GDAL/OGR

FT_INTEGER = 0
FT_REAL = 2
FT_STRING = 4 
FT_DATE = 9
FT_TIME = 10
FT_DATETIME = 11
FT_INTEGER64 = 12

# Sample WKB geometries

WKB_POINT_00 = a2b_hex('010100000000000000000000000000000000000000')  # POINT(0 0)
WKB_POINT_11 = a2b_hex('0101000000000000000000f03f000000000000f03f')  # POINT(1 1)

WKB_LINESTRING = a2b_hex('01020000000200000000000000000000000000000000000000000000000000f03f000000000000f03f')  # LINESTRING(0 0, 1 1)


@pytest.mark.xfail(reason="Not implemented yet")
def test_from_data(shared_datadir, reset_svg_paths):
    crs = CRS.from_epsg(4326)

    Layer.from_data(
        GT_POINT, crs, (
            ('f_integer', FT_INTEGER),
            ('f_real', FT_REAL),
            ('f_string', FT_STRING),
        ), (
            (1, WKB_POINT_00, (1, 0.33, 'foo')),
            (2, WKB_POINT_11, (1, 0.33, None)),
        )
    )

    empty_tuple = tuple()
    Layer.from_data(
        GT_LINESTRING, crs, empty_tuple, (
            (1, WKB_LINESTRING, empty_tuple),
            (2, WKB_LINESTRING, empty_tuple),
        )
    )