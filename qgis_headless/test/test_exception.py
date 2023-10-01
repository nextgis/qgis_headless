from qgis_headless import QgisHeadlessError, StyleTypeMismatch, StyleValidationError


def test_exception():
    geom_type_mismatch = StyleTypeMismatch("StyleTypeMismatch")
    assert isinstance(geom_type_mismatch, StyleValidationError)

    style_validation_error = StyleValidationError("StyleValidationError")
    assert isinstance(style_validation_error, QgisHeadlessError)
