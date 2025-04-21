from functools import partial

import pytest

from qgis_headless import get_qgis_version_int

xfail = partial(pytest.mark.xfail, strict=True)


class Issues:
    DIAGRAM_ATTRIBUTES = xfail(reason="https://github.com/qgis/QGIS/issues/33810")
    GRADIENT = xfail(
        condition=get_qgis_version_int() < 33200, reason="Only edge gradient colours are drawn"
    )
    UWSGI_SEGFAULT = xfail(reason="uWSGI segfault during SIGTERM")
    SVG_MARKER_NOT_RESOLVED = xfail(reason="SVG markers are not being resolved")
    WRONG_MAP_SCALE = xfail(reason="@map_scale always returns 1")
    WRONG_CALC_ELLIPSOID = xfail(reason="Wrong ellipsoid for calculations")
