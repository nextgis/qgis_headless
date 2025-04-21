import pytest

from qgis_headless import CRS, InvalidCRSError


def test_crs_from_epsg_valid():
    CRS.from_epsg(3857)
    CRS.from_epsg(4326)


def test_crs_from_epsg_invalid():
    with pytest.raises(InvalidCRSError):
        CRS.from_epsg(-1)


def test_crs_from_wkt_valid():
    CRS.from_wkt(
        'GEOGCS["WGS 84",'
        'DATUM["WGS_1984",'
        'SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],'
        'AUTHORITY["EPSG","6326"]],'
        'PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],'
        'UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],'
        'AUTHORITY["EPSG","4326"]]'
    )


def test_crs_from_wkt_invalid():
    with pytest.raises(InvalidCRSError):
        CRS.from_wkt('GEOGCRS["WGS 84",')
