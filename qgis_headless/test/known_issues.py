from functools import partial

import pytest
from osgeo import gdal

xfail = partial(pytest.mark.xfail, strict=True)


class Issues:
    UWSGI_SEGFAULT = xfail(reason="uWSGI segfault during SIGTERM")
    UWSGI_SEGFAULT_NO_STRICT = xfail(reason="uWSGI segfault during SIGTERM", strict=False)
    WRONG_FIDS = xfail(reason="Features are not assigned fids and use sequential numbers instead")
    JSON_OBJECTS_ONLY = xfail(reason="Only JSON objects are supported")
    COG_HEADER_ARE_NOT_REUSED = xfail(
        condition=int(gdal.VersionInfo("VERSION_NUM")) < 3080400,
        reason="GDAL < 3.8.4 does not support /vsicache protocol",
    )
