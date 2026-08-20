from functools import partial

import pytest

xfail = partial(pytest.mark.xfail, strict=True)


class Issues:
    JSON_OBJECTS_ONLY = xfail(reason="Only JSON objects are supported")
