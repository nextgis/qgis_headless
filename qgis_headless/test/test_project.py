from qgis_headless import (
    CRS,
    Layer,
    MapRequest,
    Project,
    Style,
)
from qgis_headless.util import image_stat, to_pil


def test_single(shared_datadir):
    project = Project.from_file(str(shared_datadir / 'projects/magadan/project.qgz'))

    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(CRS.from_epsg(4326))

    req.add_project(project)

    img = to_pil(req.render_image((150.605, 59.456, 151.116, 59.653), (256, 256)))
    stat = image_stat(img)
    assert stat.red.max == stat.green.max == 0, "Unexpected data in blue or green channel"
    assert stat.blue.max == 255, "Blue marker is missing"


def test_with_layer(shared_datadir):
    layer = Layer.from_ogr(str(shared_datadir / 'zero.geojson'))
    style = Style.from_file(str(shared_datadir / 'zero-red-circle.qml'))
    project = Project.from_file(str(shared_datadir / 'projects/magadan/project.qgz'))

    req = MapRequest()
    req.set_dpi(96)
    req.set_crs(CRS.from_epsg(3857))

    req.add_layer(layer, style)
    req.add_project(project)

    img = to_pil(req.render_image((-10000, -10000, 16822224, 8312363), (256, 256)))
    stat = image_stat(img)
    assert stat.green.max == 0, "Unexpected data in blue or green channel"
    assert stat.red.max == 255, "Layer marker is missing"
    assert stat.blue.max == 255, "Project marker is missing"


# def test_multiple(shared_datadir):
#     proj_magadan = Project.from_file(str(shared_datadir / 'projects/magadan/project.qgz'))
#     proj_vl = Project.from_file(str(shared_datadir / 'projects/vladivostok/project.qgz'))

#     req = MapRequest()
#     req.set_dpi(96)
#     req.set_crs(CRS.from_epsg(4326))

#     req.add_project(proj_magadan)
#     req.add_project(proj_vl)

#     img = to_pil(req.render_image((131.183, 42.697, 151.116, 59.653), (256, 256)))
#     img.save('share/out.png')
