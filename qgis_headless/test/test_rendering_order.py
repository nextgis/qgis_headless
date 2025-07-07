from qgis_headless import (
    CRS,
    Layer,
)
from qgis_headless.util import (
    image_stat,
    render_vector,
)

COLOR_CHECK_POINTS = ((74, 127), (147, 127))


def check_rendering_order(
    layer_image, expected_colors, layer_name, style_name, color_check_points=COLOR_CHECK_POINTS
):
    """
    Checks the colors at the given points is equal to the expected colors.

    :param layer_image: A PIL Image containing rendered layer.
    :param expected_colors: An iterable contaning expected colors at the color_check_points.
    :param layer_name: The name of layer for error output.
    :param style_name: The name of style for error output.
    :param color_check_points: Points where colors are checked.
    """

    stat = image_stat(layer_image)
    assert stat.alpha.max > 0, f"Layer {layer_name} with style {style_name} is missing"

    for point, color in zip(color_check_points, expected_colors):
        assert layer_image.getpixel(point) == color, (
            f"Incorrect rendering order of layer {layer_name} with style {style_name}"
        )


def test_single(save_img, shared_datadir):
    crs = CRS.from_epsg(4326)

    asc_layer_name = "asc.geojson"
    desc_layer_name = "desc.geojson"

    asc_style_name = "asc.qml"
    desc_style_name = "desc.qml"

    asc_extent = (0.0, 8.0, 23.3333, 16.0)
    desc_extent = (0.0, 0.0, 23.3333, 8.0)

    border_color = (255, 35, 0, 255)
    fill_color = (255, 255, 255, 255)

    asc_layer = Layer.from_ogr(shared_datadir / "rendering-order" / asc_layer_name)
    desc_layer = Layer.from_ogr(shared_datadir / "rendering-order" / desc_layer_name)

    asc_style = (shared_datadir / "rendering-order" / asc_style_name).read_text()
    desc_style = (shared_datadir / "rendering-order" / desc_style_name).read_text()

    # Direct rendering order:

    asc_image = save_img(render_vector(asc_layer, asc_style, asc_extent, crs=crs))
    desc_image = save_img(render_vector(desc_layer, desc_style, desc_extent, crs=crs))

    check_rendering_order(
        asc_image, (fill_color for _ in COLOR_CHECK_POINTS), asc_layer_name, asc_style_name
    )
    check_rendering_order(
        desc_image, (fill_color for _ in COLOR_CHECK_POINTS), desc_layer_name, desc_style_name
    )

    # Inverted rendering order:

    asc_image = save_img(render_vector(asc_layer, desc_style, asc_extent, crs=crs))
    desc_image = save_img(render_vector(desc_layer, asc_style, desc_extent, crs=crs))

    check_rendering_order(
        asc_image, (border_color for _ in COLOR_CHECK_POINTS), asc_layer_name, desc_style_name
    )
    check_rendering_order(
        desc_image, (border_color for _ in COLOR_CHECK_POINTS), desc_layer_name, asc_style_name
    )


def test_categorized(save_img, shared_datadir):
    crs = CRS.from_epsg(4326)

    asc_layer_name = "asc.geojson"
    desc_layer_name = "desc.geojson"

    asc_style_name = "asc-categorized.qml"
    desc_style_name = "desc-categorized.qml"

    asc_extent = (0.0, 8.0, 23.3333, 16.0)
    desc_extent = (0.0, 0.0, 23.3333, 8.0)

    # Colors of triangles:
    brown = (203, 129, 92, 255)
    purple = (192, 14, 223, 255)
    blue = (75, 182, 236, 255)

    asc_expected_colors = (brown, purple)
    desc_expected_colors = (blue, purple)
    asc_expected_colors_inverted = (purple, blue)
    desc_expected_colors_inverted = (purple, brown)

    asc_layer = Layer.from_ogr(shared_datadir / "rendering-order" / asc_layer_name)
    desc_layer = Layer.from_ogr(shared_datadir / "rendering-order" / desc_layer_name)

    asc_style = (shared_datadir / "rendering-order" / asc_style_name).read_text()
    desc_style = (shared_datadir / "rendering-order" / desc_style_name).read_text()

    # Direct rendering order:

    asc_image = save_img(render_vector(asc_layer, asc_style, asc_extent, crs=crs))
    desc_image = save_img(render_vector(desc_layer, desc_style, desc_extent, crs=crs))

    check_rendering_order(asc_image, asc_expected_colors, asc_layer_name, desc_style_name)
    check_rendering_order(desc_image, desc_expected_colors, desc_layer_name, desc_style_name)

    # Inverted rendering order:

    asc_image = save_img(render_vector(asc_layer, desc_style, asc_extent, crs=crs))
    desc_image = save_img(render_vector(desc_layer, asc_style, desc_extent, crs=crs))

    check_rendering_order(asc_image, asc_expected_colors_inverted, asc_layer_name, desc_style_name)
    check_rendering_order(
        desc_image, desc_expected_colors_inverted, desc_layer_name, desc_style_name
    )
