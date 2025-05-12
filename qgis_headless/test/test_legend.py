from typing import Any, Dict

import pytest
from packaging import version

from qgis_headless import (
    CRS,
    LT_VECTOR,
    Layer,
    MapRequest,
    QgisHeadlessError,
    Style,
    get_qgis_version,
    set_svg_paths,
)
from qgis_headless.util import (
    image_stat,
    is_same_color,
    render_legend,
    to_pil,
)

QGIS_VERSION = version.parse(get_qgis_version().split("-")[0])


def test_legend(save_img, shared_datadir, reset_svg_paths):
    data = shared_datadir / "contour/data.geojson"
    style = (shared_datadir / "contour/rgb.qml").read_text()

    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(CRS.from_epsg(3857))

    req.add_layer(Layer.from_ogr(data), Style.from_string(style), label="Contour")

    rendered_legend = req.render_legend()
    img = save_img(to_pil(rendered_legend))

    EXPECTED_WIDTH = 224
    EXPECTED_HEIGHT = 92
    assert (
        (EXPECTED_WIDTH - 2, EXPECTED_HEIGHT - 2)
        <= img.size
        <= (EXPECTED_WIDTH + 2, EXPECTED_HEIGHT + 2)
    ), f"Expected size is ~{EXPECTED_WIDTH}x{EXPECTED_HEIGHT}"

    stat = image_stat(img)
    assert stat.green.max == 255, "Primary lines aren't visible"
    assert stat.blue.max == 255, "Primary lines aren't visible"
    assert stat.red.mean == 0
    assert 1 < stat.green.mean < 3
    assert 1 < stat.blue.mean < 3

    req.set_dpi(2 * 96)
    rendered_legend = req.render_legend()
    hdpi_img = to_pil(rendered_legend)

    assert img.size[0] < hdpi_img.size[0] and img.size[1] < hdpi_img.size[1], (
        "Higher DPI should produce bigger legend"
    )


RED = (255, 0, 0, 255)
GREEN = (0, 255, 0, 255)
BLUE = (0, 0, 255, 255)
TRANSPARENT = (0, 0, 0, 0)
GRAY_128 = (128, 128, 128, 128)

LEGEND_DEFAULT_SIZES = [20, 40]

legend_symbols_params = []
for id, geometry_type, style, sizes, expected in [
    (
        "zero_red_circle",
        Layer.GT_POINT,
        dict(file="zero/red-circle.qml"),
        LEGEND_DEFAULT_SIZES + [24, 32, (32, 24)],
        [
            (None, dict({k: TRANSPARENT for k in ("lt", "rt", "lb", "rb")}, c=RED)),
        ],
    ),
    (
        "default_point",
        Layer.GT_POINT,
        dict(color=GRAY_128),
        LEGEND_DEFAULT_SIZES,
        ((None, dict({k: TRANSPARENT for k in ("lt", "rt", "lb", "rb")}, c=GRAY_128)),),
    ),
    (
        "default_linestring",
        Layer.GT_LINESTRING,
        dict(color=GRAY_128),
        LEGEND_DEFAULT_SIZES,
        ((None, dict(ct=TRANSPARENT, cb=TRANSPARENT)),),
    ),
    (
        "default_polygon",
        Layer.GT_POLYGON,
        dict(color=GRAY_128),
        LEGEND_DEFAULT_SIZES,
        ((None, GRAY_128),),
    ),
    (
        "contour_rgb",
        Layer.GT_LINESTRING,
        dict(file="contour/rgb.qml"),
        LEGEND_DEFAULT_SIZES + [24, 32, (32, 24)],
        (
            ("primary horizontals", dict(c=GREEN, ct=TRANSPARENT, cb=TRANSPARENT)),
            ("secondary horizontals", dict(c=BLUE, ct=TRANSPARENT, cb=TRANSPARENT)),
        ),
    ),
    (
        "thick_outline",
        Layer.GT_POLYGON,
        dict(file="thick-outline.qml"),
        LEGEND_DEFAULT_SIZES,
        ((None, dict()),),
    ),
    (
        "big_symbol",
        Layer.GT_POINT,
        dict(file="big-symbol.qml"),
        LEGEND_DEFAULT_SIZES,
        ((None, dict()),),
    ),
    (
        "bar_chart",
        Layer.GT_POLYGON,
        dict(file="diagram/bar-chart.qml"),
        LEGEND_DEFAULT_SIZES + [24, 32, (32, 24)],
        (
            ("", dict(c=TRANSPARENT, ct=TRANSPARENT, cb=TRANSPARENT)),
            ("red", dict(c=RED, ct=TRANSPARENT, cb=TRANSPARENT)),
            ("green", dict(c=GREEN, ct=TRANSPARENT, cb=TRANSPARENT)),
            ("blue", dict(c=BLUE, ct=TRANSPARENT, cb=TRANSPARENT)),
        ),
    ),
    (
        "pie_chart",
        Layer.GT_POLYGON,
        dict(file="diagram/pie-chart.qml"),
        LEGEND_DEFAULT_SIZES + [24, 32, (32, 24)],
        (
            ("", dict(c=TRANSPARENT, ct=TRANSPARENT, cb=TRANSPARENT)),
            ("red", dict(c=RED, ct=TRANSPARENT, cb=TRANSPARENT)),
            ("green", dict(c=GREEN, ct=TRANSPARENT, cb=TRANSPARENT)),
            ("blue", dict(c=BLUE, ct=TRANSPARENT, cb=TRANSPARENT)),
        ),
    ),
]:
    for size in sizes:
        if isinstance(size, int):
            size = (size, size)
        param_id = f"{id}-{size[0]}x{size[1]}"
        legend_symbols_params.append(
            pytest.param(geometry_type, style, size, expected, id=param_id)
        )


@pytest.mark.parametrize("geometry_type, style_params, size, expected", legend_symbols_params)
def test_legend_symbols(geometry_type, style_params, size, expected, save_img, shared_datadir):
    if "file" in style_params:
        style = Style.from_file(shared_datadir / style_params["file"])
    else:
        style = Style.from_defaults(
            layer_type=LT_VECTOR,
            layer_geometry_type=geometry_type,
            color=style_params["color"],
        )

    req = MapRequest()
    req.set_dpi(96)
    req.add_layer(Layer.from_data(geometry_type, CRS.from_epsg(3857), (), ()), style)

    symbols = req.legend_symbols(0, size)
    symbols_count = len(symbols)
    expected_count = len(expected)
    assert symbols_count == expected_count, "count mismatch"

    for symbol, (expected_title, expected_colors) in zip(symbols, expected):
        assert symbol.title() == expected_title, "title mismatch"

        image = save_img(to_pil(symbol.icon()), symbol.title() or "symbol")
        im_size = image.size

        im_width, im_height = im_size
        assert im_width != 0, "zero width"
        assert im_height != 0, "zero height"

        sides_coords = (
            dict(l=0, r=im_width - 1, c=(im_width - 1) // 2),
            dict(t=0, b=im_height - 1, c=(im_height - 1) // 2),
        )

        if isinstance(expected_colors, tuple):
            expected_colors = dict(c=expected_colors)

        for position, expected_color in expected_colors.items():
            if position == "c":
                position = "cc"

            pixel_coord = tuple(
                side_coords[side] for side, side_coords in zip(position, sides_coords)
            )
            pixels_coord = [pixel_coord]
            if position == "cc":
                pixels_coord.append((pixel_coord[0], pixel_coord[1] - 1))
                pixels_coord.append((pixel_coord[0], pixel_coord[1] + 1))
            pixels_color = [image.getpixel(pixel_coord) for pixel_coord in pixels_coord]

            assert any(
                is_same_color(pixel_color, expected_color) for pixel_color in pixels_color
            ), f"{position}{pixel_coord} color mismatch: {expected_color} != {pixels_color[0]} "

        assert im_size == size, f"size mismatch: {im_size} != {size}"


@pytest.mark.parametrize(
    "style_file, layer_file, extent, cases",
    (
        (
            "landuse/landuse.qml",
            "landuse/landuse.geojson",
            (4189314.0, 7505071.0, 4190452.0, 7506101.0),
            (
                ((-1,), dict(exc=QgisHeadlessError)),
                ((2,), dict(exc=QgisHeadlessError)),
                (None, dict(colors=(255, 255, 0, None))),
                ((0, 1), dict(colors=(255, 255, 0, None))),
                ((), dict(colors=(None, None, None, 0))),
                ((0,), dict(colors=(255, 0, 0, 255))),
                ((1,), dict(colors=(0, 255, 0, 255))),
            ),
        ),
        (
            "categories/rgb.qml",
            "categories/rgb.geojson",
            (-4400, -14000, 4400, 14000),
            (
                (None, dict(colors=(255, 0, 255, 255))),
                ((0,), dict(colors=(255, 0, 0, 255))),
                ((1,), dict(colors=(0, 255, 0, 255))),
                ((2,), dict(colors=(0, 0, 255, 255))),
            ),
        ),
        pytest.param(
            "categories/null-attr.qml",
            "categories/null-attr.geojson",
            (-4400, -14000, 4400, 14000),
            (
                (None, dict(colors=(255, 0, 255, 255))),
                ((0,), dict(colors=(255, 0, 0, 255))),
                ((1,), dict(colors=(0, 0, 255, 255))),
            ),
            marks=pytest.mark.skipif(
                QGIS_VERSION < version.parse("3.36"),
                reason="QML category syntax changing",
            ),
        ),
    ),
)
def test_legend_symbols_render(style_file, layer_file, extent, cases, shared_datadir):
    style = Style.from_file(shared_datadir / style_file)
    layer = Layer.from_ogr(shared_datadir / layer_file)

    req = MapRequest()
    req.set_dpi(96)
    req.add_layer(layer, style)
    req.set_crs(CRS.from_epsg(3857))

    for symbols, expected in cases:
        params: Dict[str, Any] = dict(extent=extent, size=(256, 256))
        if symbols is not None:
            params["symbols"] = ((0, symbols),)
        if exc := expected.get("exc"):
            with pytest.raises(exc):
                req.render_image(**params)
        else:
            img = to_pil(req.render_image(**params))
            stat = image_stat(img)
            for band, band_max in zip(("red", "green", "blue", "alpha"), expected["colors"]):
                if band_max is not None:
                    assert getattr(stat, band).max == band_max, f"{band} color mismatch"


def test_legend_svg_path(save_img, shared_datadir, reset_svg_paths):
    data_path = shared_datadir / "zero/data.geojson"
    style_path = shared_datadir / "zero/marker.qml"

    set_svg_paths([str(shared_datadir / "marker-blue")])

    stat = image_stat(save_img(render_legend(data_path, style_path, "Marker")))
    assert stat.blue.max == 255, "Blue marker is missing"
