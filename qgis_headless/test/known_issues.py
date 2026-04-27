from functools import partial

import pytest

xfail = partial(pytest.mark.xfail, strict=True)


class Issues:
    UWSGI_SEGFAULT = xfail(reason="uWSGI segfault during SIGTERM")
    UWSGI_SEGFAULT_NO_STRICT = xfail(reason="uWSGI segfault during SIGTERM", strict=False)
    WRONG_FIDS = xfail(reason="Features are not assigned fids and use sequential numbers instead")
