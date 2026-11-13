from functools import partial

import pytest

xfail = partial(pytest.mark.xfail, strict=True)


class Issues:
    UWSGI_SEGFAULT = xfail(reason="uWSGI segfault during SIGTERM")
    UWSGI_SEGFAULT_NO_STRICT = xfail(reason="uWSGI segfault during SIGTERM", strict=False)
    WRONG_FIDS = xfail(reason="Features are not assigned fids and use sequential numbers instead")
    JSON_OBJECTS_ONLY = xfail(reason="Only JSON objects are supported")
    LEGEND_SYMBOL_CLIPPING = xfail(
        reason="Direct symbol previews do not restrict oversized symbols"
    )
    LEGEND_LAYER_OPACITY = xfail(reason="Direct symbol previews ignore layer opacity")
    LEGEND_COLLAPSED_SIZE = xfail(
        reason="Collapsed size legends have no QgsSymbol and lose their icon"
    )
    LEGEND_EXPRESSION_SCOPE = xfail(
        reason="Legend rendering omits global, project and layer scopes"
    )
    LEGEND_EXPRESSION_OPACITY = xfail(
        reason="Symbol preview does not evaluate data-defined opacity"
    )
