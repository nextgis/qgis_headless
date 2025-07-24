from functools import partial

import pytest

from qgis_headless import get_qgis_version_int

xfail = partial(pytest.mark.xfail, strict=True)


class Issues:
    DIAGRAM_ATTRIBUTES = xfail(
        condition=get_qgis_version_int() < 33000,
        reason="https://github.com/qgis/QGIS/issues/33810",
    )
    DIAGRAM_SYMBOLS = pytest.mark.xfail(
        condition=get_qgis_version_int() <= 32216,
        reason="Rendering of diagram legend symbols is incorrect in QGIS 2.20 and older",
    )
    GRADIENT = xfail(
        condition=get_qgis_version_int() < 33200, reason="Only edge gradient colours are drawn"
    )
    UWSGI_SEGFAULT = xfail(reason="uWSGI segfault during SIGTERM")
    SVG_MARKER_NOT_RESOLVED = xfail(reason="SVG markers are not being resolved")
    WRONG_FIDS = xfail(
        condition=get_qgis_version_int() < 33000,
        reason="Features are not assigned fids and use sequential numbers instead",
    )
