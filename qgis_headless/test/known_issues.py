from functools import partial

import pytest

xfail = partial(pytest.mark.xfail, strict=True)


class Issues:
    WRONG_FIDS = xfail(reason="Features are not assigned fids and use sequential numbers instead")
    JSON_OBJECTS_ONLY = xfail(reason="Only JSON objects are supported")
