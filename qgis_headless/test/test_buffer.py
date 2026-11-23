import gc

import pytest

from qgis_headless import CRS, MapRequest, RawData
from qgis_headless.util import EXTENT_ONE


@pytest.mark.parametrize("direct", [False, True], ids=["to-bytes", "buffer-protocol"])
def test_image_buffer_owns_storage(direct):
    request = MapRequest()
    request.set_crs(CRS.from_epsg(3857))
    image = request.render_image(EXTENT_ONE, (32, 32))
    view = memoryview(image) if direct else image.to_bytes()
    assert view.readonly
    assert view.format == "B"
    assert view.shape == (32 * 32 * 4,)
    assert view.strides == (1,)
    expected = bytes(view)
    del image, request
    gc.collect()
    assert bytes(view) == expected
    with pytest.raises(TypeError):
        view[0] = 1
    sliced = view[::2]
    view.release()
    del view
    gc.collect()
    assert bytes(sliced) == expected[::2]


@pytest.mark.parametrize("direct", [False, True], ids=["to-bytes", "buffer-protocol"])
def test_raw_data_buffer_owner(direct):
    data = RawData()
    view = memoryview(data) if direct else data.to_bytes()
    assert view.readonly
    del data
    gc.collect()
    assert bytes(view) == b""
