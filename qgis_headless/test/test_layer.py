from datetime import UTC, date, datetime, time
from subprocess import CalledProcessError, check_call
from sys import executable
from textwrap import dedent
from xml.sax.saxutils import quoteattr

import pytest

from qgis_headless import CRS, InvalidLayerSource, Layer, Style
from qgis_headless.test.known_issues import Issues
from qgis_headless.util import (
    EXTENT_ONE,
    WKB_LINESTRING,
    WKB_POINT_00,
    WKB_POINT_11,
    WKB_POINTZ_000,
    WKB_POINTZ_111,
    image_stat,
    render_vector,
)


@pytest.mark.parametrize(
    "gt, pt0, pt1",
    (
        pytest.param(Layer.GT_POINT, WKB_POINT_00, WKB_POINT_11, id="2D"),
        pytest.param(Layer.GT_POINTZ, WKB_POINTZ_000, WKB_POINTZ_111, id="3D"),
    ),
)
def test_from_data_red_circle(gt, pt0, pt1, shared_datadir, reset_svg_paths):
    style = (shared_datadir / "zero/red-circle.qml").read_text()
    crs = CRS.from_epsg(3857)

    layer = Layer.from_data(
        gt,
        crs,
        (
            ("f_integer", Layer.FT_INTEGER),
            ("f_real", Layer.FT_REAL),
            ("f_string", Layer.FT_STRING),
        ),
        (
            (1, pt0, (1, 0.33, "foo")),
            (2, pt1, (1, None, "bar")),
        ),
    )

    img = render_vector(layer, style, EXTENT_ONE, 256)
    # img.save("test_from_data_red_circle.png")

    stat = image_stat(img)
    assert stat.red.max == 255, "Red marker missing"


@pytest.mark.parametrize(
    "field_type, value",
    [
        (Layer.FT_DATETIME, datetime(2020, 1, 1, microsecond=1)),
        (Layer.FT_DATETIME, datetime(2020, 1, 1, tzinfo=UTC)),
        (Layer.FT_TIME, time(microsecond=1)),
        (Layer.FT_TIME, time(tzinfo=UTC)),
    ],
)
def test_datetime_precision_rejected(field_type, value):
    with pytest.raises(ValueError):
        Layer.from_data(
            Layer.GT_POINT,
            CRS.from_epsg(3857),
            (("field", field_type),),
            ((1, WKB_POINT_00, (value,)),),
        )


@pytest.mark.parametrize("values", [(), (1, 2)])
@pytest.mark.parametrize("attribute_container", [tuple, iter], ids=["tuple", "iterator"])
def test_from_data_attribute_count(values, attribute_container):
    with pytest.raises(ValueError, match="attribute count"):
        Layer.from_data(
            Layer.GT_POINT,
            CRS.from_epsg(3857),
            (("field", Layer.FT_INTEGER),),
            ((1, WKB_POINT_00, attribute_container(values)),),
        )


@pytest.mark.parametrize(
    "field_type,value,error",
    [
        pytest.param(Layer.FT_DATE, (2020, 1), "Date requires", id="date"),
        pytest.param(Layer.FT_TIME, (12, 30), "Time requires", id="time"),
        pytest.param(Layer.FT_DATETIME, (2020, 1, 1, 12, 30), "Datetime requires", id="datetime"),
    ],
)
def test_from_data_incomplete_calendar_tuple(field_type, value, error):
    with pytest.raises(ValueError, match=error):
        Layer.from_data(
            Layer.GT_POINT,
            CRS.from_epsg(3857),
            (("field", field_type),),
            ((1, WKB_POINT_00, (value,)),),
        )


def test_from_data_incomplete_field_definition():
    with pytest.raises(ValueError, match="Field requires a name and type"):
        Layer.from_data(Layer.GT_POINT, CRS.from_epsg(3857), (("field",),), ())


@pytest.mark.parametrize("depth", [64, 65])
@pytest.mark.parametrize("leaf", [{}, [], 42])
def test_json_nesting_limit(depth, leaf):
    value = leaf
    for _ in range(depth - isinstance(leaf, (dict, list))):
        value = {"child": value}

    def create_layer():
        return Layer.from_data(
            Layer.GT_POINT,
            CRS.from_epsg(3857),
            (("field", Layer.FT_JSON),),
            ((1, WKB_POINT_00, (value,)),),
        )

    if depth == 64:
        create_layer()
    else:
        with pytest.raises(ValueError, match="nesting"):
            create_layer()


def test_json_cycle_rejected():
    value = {}
    value["self"] = value
    with pytest.raises(ValueError, match="nesting"):
        Layer.from_data(
            Layer.GT_POINT,
            CRS.from_epsg(3857),
            (("field", Layer.FT_JSON),),
            ((1, WKB_POINT_00, (value,)),),
        )


def test_from_data_generator():
    def features():
        yield (1, WKB_POINT_00, ())

    layer = Layer.from_data(Layer.GT_POINT, CRS.from_epsg(3857), (), features())
    image = render_vector(layer, Style.from_defaults(color=(255, 0, 0, 255)), EXTENT_ONE)
    assert image.getchannel("A").getbbox() is not None


def test_from_data_provider_rejection():
    with pytest.raises(ValueError, match="Cannot insert feature"):
        Layer.from_data(Layer.GT_POINT, CRS.from_epsg(3857), (), ((1, WKB_LINESTRING, ()),))


@pytest.mark.parametrize("fid", [0, 8, 2**63 - 1])
def test_from_data_duplicate_fid(fid):
    rows = ((fid, WKB_POINT_00, ()) for _ in range(2))
    with pytest.raises(ValueError, match=f"Feature with ID {fid} already exists"):
        Layer.from_data(Layer.GT_POINT, CRS.from_epsg(3857), (), rows)


@pytest.mark.parametrize(
    "fid, expected", [(0, 0), (8, 8), (2**63 - 1, 2**63 - 1), (-1, 1), (-(2**63), 1)]
)
def test_from_data_fid_assignment(fid, expected, shared_datadir, reset_svg_paths):
    style = (shared_datadir / "zero/placeholder.qml").read_text()
    style = style.replace('"/* {{{ */ FALSE /* }}} */"', quoteattr(f"@id = {expected}"))
    rows = iter(((fid, WKB_POINT_00, ()),))
    layer = Layer.from_data(Layer.GT_POINT, CRS.from_epsg(3857), (), rows)
    image = render_vector(layer, style, EXTENT_ONE, 256)
    assert image_stat(image).red.max == 255, f"Wrong @id for input FID {fid}"


def test_from_data_fid_exhaustion():
    rows = iter(((2**63 - 1, WKB_POINT_00, ()), (-1, WKB_POINT_00, ())))
    with pytest.raises(ValueError, match="maximum ID has been reached"):
        Layer.from_data(Layer.GT_POINT, CRS.from_epsg(3857), (), rows)


def test_from_data_linestring(shared_datadir, reset_svg_paths):
    crs = CRS.from_epsg(3857)
    empty_tuple = ()
    Layer.from_data(
        Layer.GT_LINESTRING,
        crs,
        empty_tuple,
        (
            (1, WKB_LINESTRING, empty_tuple),
            (2, WKB_LINESTRING, empty_tuple),
        ),
    )


@pytest.mark.parametrize("attribute_container", [tuple, iter], ids=["tuple", "iterator"])
@pytest.mark.parametrize(
    "ftype, fvalue, cond",
    (
        pytest.param(Layer.FT_INTEGER, None, "field IS NULL", id="integer-null"),
        pytest.param(Layer.FT_INTEGER, 42, "field = 42", id="integer-42"),
        pytest.param(Layer.FT_REAL, 0.25, "field = 0.25", id="real"),
        pytest.param(Layer.FT_STRING, "", "field = ''", id="string-empty"),
        pytest.param(Layer.FT_STRING, "0" * 512, "length(field) = 512", id="string-512"),
        pytest.param(Layer.FT_STRING, "съёмка 🌍", "field = 'съёмка 🌍'", id="string-unicode"),
        pytest.param(
            Layer.FT_STRING, "a\0б", "length(field) = 3 AND right(field, 1) = 'б'", id="string-nul"
        ),
        pytest.param(Layer.FT_STRING, "текст".encode(), "field = 'текст'", id="string-utf8-bytes"),
        pytest.param(
            Layer.FT_STRING,
            bytearray("текст", "utf-8"),
            "field = 'текст'",
            id="string-utf8-bytearray",
        ),
        pytest.param(Layer.FT_DATE, (1917, 11, 7), "field = to_date('1917-11-07')", id="date"),
        pytest.param(
            Layer.FT_DATE, date(1600, 1, 2), "field = to_date('1600-01-02')", id="native-date"
        ),
        pytest.param(
            Layer.FT_DATE, date(9999, 12, 31), "year(field) = 9999", id="native-date-max"
        ),
        pytest.param(Layer.FT_TIME, time(16, 20), "field = to_time('16:20:00')", id="native-time"),
        pytest.param(
            Layer.FT_TIME,
            time(16, 20, 0, 123000),
            "field = to_time('16:20:00.123', 'hh:mm:ss.zzz')",
            id="native-time-milliseconds",
        ),
        pytest.param(
            Layer.FT_DATETIME,
            datetime(1800, 1, 2, 3, 4, 5),
            "field = to_datetime('1800-01-02 03:04:05')",
            id="native-datetime",
        ),
        pytest.param(
            Layer.FT_DATETIME,
            datetime(2020, 1, 2, 3, 4, 5, 123000),
            "field = to_datetime('2020-01-02 03:04:05.123', 'yyyy-MM-dd hh:mm:ss.zzz')",
            id="native-datetime-milliseconds",
        ),
        pytest.param(Layer.FT_TIME, (16, 20, 0), "field = to_time('16:20:00')", id="time"),
        pytest.param(
            Layer.FT_DATETIME,
            (1917, 11, 7, 16, 20, 0),
            "field = to_datetime('1917-11-07 16:20:00')",
            id="datetime",
        ),
        pytest.param(
            Layer.FT_INTEGER64,
            2**63 - 1,
            "field = 9223372036854775807",
            id="integer64-max",
        ),
        pytest.param(Layer.FT_BOOLEAN, True, "field", id="boolean-true"),
        pytest.param(Layer.FT_BOOLEAN, False, "NOT field", id="boolean-false"),
        pytest.param(
            Layer.FT_JSON,
            '{"foo": "bar"}',
            "map_get(field, 'foo') = 'bar'",
            id="json-object-map_get",
        ),
        pytest.param(
            Layer.FT_JSON,
            {"foo": {"bar": [True, None, 42, "текст"]}},
            "field['foo']['bar'][0] AND field['foo']['bar'][1] IS NULL AND field['foo']['bar'][2] = 42 AND field['foo']['bar'][3] = 'текст'",
            id="json-native-nested",
        ),
        pytest.param(
            Layer.FT_JSON,
            {"метка 🌍": "a\0б"},
            "length(field['метка 🌍']) = 3 AND right(field['метка 🌍'], 1) = 'б'",
            id="json-native-unicode-key-and-nul",
        ),
        pytest.param(
            Layer.FT_JSON,
            {"value": 2**63 - 1},
            "field['value'] = 9223372036854775807",
            id="json-native-integer64",
        ),
        pytest.param(
            Layer.FT_JSON,
            '{"foo": "bar"}',
            "field['foo'] = 'bar'",
            id="json-object-subscript",
        ),
        pytest.param(
            Layer.FT_JSON,
            '{"foo": {"bar": "qux"}}',
            "field['foo']['bar'] = 'qux'",
            id="json-object-nested",
        ),
        pytest.param(
            Layer.FT_JSON,
            '["foo", "bar"]',
            "field[1] = 'bar'",
            id="json-array-index",
            marks=Issues.JSON_OBJECTS_ONLY,
        ),
    ),
)
def test_field_type(ftype, fvalue, cond, shared_datadir, reset_svg_paths, attribute_container):
    style = (shared_datadir / "zero/placeholder.qml").read_text()
    crs = CRS.from_epsg(3857)

    layer = Layer.from_data(
        Layer.GT_POINT,
        crs,
        (("field", ftype),),
        ((1, WKB_POINT_00, attribute_container((fvalue,))),),
    )

    style = style.replace('"/* {{{ */ FALSE /* }}} */"', quoteattr(cond))

    img = render_vector(layer, style, EXTENT_ONE, 256)
    # img.save("test_field_type.png")

    stat = image_stat(img)
    assert stat.red.max == 255, "Red marker missing"


def test_geometry_crash():
    total, failed = 0, 0
    for _ in range(10):
        try:
            total += 1
            # fmt: off
            check_call([executable, "-c", dedent("""
                from qgis_headless import Layer, CRS, init
                from binascii import a2b_hex

                FEATURE = (
                    0,
                    a2b_hex(
                        '01050000000700000001020000000400000029ea4a6e095d53414db60523'
                        '9f97574132c260fb255e5341915becb3d0965741eff4e1d4c05f534142bf'
                        'ecd11c92574154aa21e12a615341e696cbbe039057410102000000090000'
                        '006911944e2464534113073323fe8c5741491253953a655341d27e51ae51'
                        '8c57410bdf11d5d8655341a9a205edf38b5741f634a7028b6853413f1c58'
                        'a9df8b5741537415b3276a5341c03f2f0f458b57419a24a933f46a534127'
                        '08823c8b8a5741c71e30939b6e5341f388a44a2f885741196219651e7153'
                        '417167bdffd18657419b0045bc9e735341078350cf318657410102000000'
                        '0300000046705071b078534148e91cf0aa8357413c6ee15af37953416b7d'
                        'ef659f825741b897b00d347b534180cf8d36a28057410102000000050000'
                        '000ad82e71508453416259b080737c5741fda13e1bdc8553410eaa9d2975'
                        '7b574145365d301c875341809fb78983785741a2b52f3d788753414ad50f'
                        'cecb775741b9437fafef8753417fe3acd51f765741010200000003000000'
                        'c53d8d31c4945341979ac9bb886b5741681b7bc4d8955341a258c526e868'
                        '5741f5c976116498534102295362556757410102000000020000004bac59'
                        '411f9b5341beeab3fab66257414caf978cd69b5341329614c1f160574101'
                        '02000000020000002c04dc9cc2a75341518af7c135365741f2cc3954e7a8'
                        '534130b2316fcd355741'
                    ), (),
                )

                init([])

                layer = Layer.from_data(Layer.GT_MULTILINESTRING, CRS.from_epsg(3857), (), (FEATURE, ))
            """)])
            # fmt: on
        except CalledProcessError:
            failed += 1

    assert failed == 0, f"Failed for {failed} times of {total}"


def test_wrong_source(shared_datadir):
    with pytest.raises(InvalidLayerSource):
        Layer.from_ogr(shared_datadir / "raster" / "rounds.tif")
    with pytest.raises(InvalidLayerSource):
        Layer.from_gdal(shared_datadir / "poly.geojson")


def test_no_geom(shared_datadir):
    Layer.from_ogr(shared_datadir / "no-geom/no-geom.geojson")
