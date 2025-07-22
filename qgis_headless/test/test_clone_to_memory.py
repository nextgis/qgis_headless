import pytest

from qgis_headless import Layer, LayerTypeMismatch


@pytest.mark.parametrize(
    "file",
    (
        pytest.param("zero/data.geojson"),
        pytest.param("line.geojson"),
        pytest.param("rough_australia.geojson"),
    ),
)
def test_clone_vector(file, shared_datadir):
    file_layer = Layer.from_ogr(shared_datadir / file)
    file_layer.clone_to_memory()


def test_clone_raster(shared_datadir):
    file_layer = Layer.from_gdal(shared_datadir / "raster/rounds.tif")
    with pytest.raises(LayerTypeMismatch):
        file_layer.clone_to_memory()
