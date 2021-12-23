from qgis_headless import (
    QgisHeadlessError,
    StyleValidationError,
    GeometryTypeMismatch,
)


def test_exception():
    geom_type_mismatch = GeometryTypeMismatch('GeometryTypeMismatch')
    assert isinstance(geom_type_mismatch, StyleValidationError)

    style_validation_error = StyleValidationError('StyleValidationError')
    assert isinstance(style_validation_error, QgisHeadlessError)
