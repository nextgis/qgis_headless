from __future__ import annotations

import collections.abc
import typing

__all__ = [
    "CRITICAL",
    "CRS",
    "DEBUG",
    "INFO",
    "Image",
    "InvalidCRSError",
    "InvalidLayerSource",
    "LT_RASTER",
    "LT_UNKNOWN",
    "LT_VECTOR",
    "Layer",
    "LayerType",
    "LegendSymbol",
    "LogLevel",
    "MapRequest",
    "Project",
    "QgisHeadlessError",
    "RawData",
    "SF_QML",
    "SF_SLD",
    "Style",
    "StyleFormat",
    "StyleTypeMismatch",
    "StyleValidationError",
    "WARNING",
    "deinit",
    "get_qgis_version",
    "get_qgis_version_int",
    "get_svg_paths",
    "get_version",
    "init",
    "set_logging_level",
    "set_svg_paths",
]

class CRS:
    @staticmethod
    def from_epsg(epsg: typing.SupportsInt) -> CRS: ...
    @staticmethod
    def from_wkt(wkt: str) -> CRS: ...
    def __init__(self) -> None: ...

class Image:
    def size(self) -> tuple[int, int]: ...
    def to_bytes(self) -> memoryview: ...

class InvalidCRSError(QgisHeadlessError):
    pass

class InvalidLayerSource(QgisHeadlessError):
    pass

class Layer:
    class AttributeType:
        """
        Members:

          FT_INTEGER

          FT_REAL

          FT_STRING

          FT_DATE

          FT_TIME

          FT_DATETIME

          FT_INTEGER64

          FT_BOOLEAN

          FT_JSON
        """

        FT_BOOLEAN: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_BOOLEAN: 7>
        FT_DATE: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_DATE: 3>
        FT_DATETIME: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_DATETIME: 5>
        FT_INTEGER: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_INTEGER: 0>
        FT_INTEGER64: typing.ClassVar[
            Layer.AttributeType
        ]  # value = <AttributeType.FT_INTEGER64: 6>
        FT_JSON: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_JSON: 8>
        FT_REAL: typing.ClassVar[Layer.AttributeType]
        FT_STRING: typing.ClassVar[Layer.AttributeType]
        FT_TIME: typing.ClassVar[Layer.AttributeType]
        __members__: typing.ClassVar[
            dict[str, Layer.AttributeType]
        ]  # value = {'FT_INTEGER': <AttributeType.FT_INTEGER: 0>, 'FT_REAL': <AttributeType.FT_REAL: 1>, 'FT_STRING': <AttributeType.FT_STRING: 2>, 'FT_DATE': <AttributeType.FT_DATE: 3>, 'FT_TIME': <AttributeType.FT_TIME: 4>, 'FT_DATETIME': <AttributeType.FT_DATETIME: 5>, 'FT_INTEGER64': <AttributeType.FT_INTEGER64: 6>, 'FT_BOOLEAN': <AttributeType.FT_BOOLEAN: 7>, 'FT_JSON': <AttributeType.FT_JSON: 8>}
        def __eq__(self, other: typing.Any) -> bool: ...
        def __getstate__(self) -> int: ...
        def __hash__(self) -> int: ...
        def __index__(self) -> int: ...
        def __init__(self, value: typing.SupportsInt) -> None: ...
        def __int__(self) -> int: ...
        def __ne__(self, other: typing.Any) -> bool: ...
        def __repr__(self) -> str: ...
        def __setstate__(self, state: typing.SupportsInt) -> None: ...
        def __str__(self) -> str: ...
        @property
        def name(self) -> str: ...
        @property
        def value(self) -> int: ...

    class GeometryType:
        """
        Members:

          GT_POINT

          GT_LINESTRING

          GT_POLYGON

          GT_MULTIPOINT

          GT_MULTILINESTRING

          GT_MULTIPOLYGON

          GT_POINTZ

          GT_LINESTRINGZ

          GT_POLYGONZ

          GT_MULTIPOINTZ

          GT_MULTILINESTRINGZ

          GT_MULTIPOLYGONZ

          GT_NULL

          GT_UNKNOWN
        """

        GT_LINESTRING: typing.ClassVar[Layer.GeometryType]
        GT_LINESTRINGZ: typing.ClassVar[Layer.GeometryType]
        GT_MULTILINESTRING: typing.ClassVar[Layer.GeometryType]
        GT_MULTILINESTRINGZ: typing.ClassVar[Layer.GeometryType]
        GT_MULTIPOINT: typing.ClassVar[Layer.GeometryType]
        GT_MULTIPOINTZ: typing.ClassVar[Layer.GeometryType]
        GT_MULTIPOLYGON: typing.ClassVar[Layer.GeometryType]
        GT_MULTIPOLYGONZ: typing.ClassVar[Layer.GeometryType]
        GT_NULL: typing.ClassVar[Layer.GeometryType]
        GT_POINT: typing.ClassVar[Layer.GeometryType]
        GT_POINTZ: typing.ClassVar[Layer.GeometryType]
        GT_POLYGON: typing.ClassVar[Layer.GeometryType]
        GT_POLYGONZ: typing.ClassVar[Layer.GeometryType]
        GT_UNKNOWN: typing.ClassVar[Layer.GeometryType]
        __members__: typing.ClassVar[dict[str, Layer.GeometryType]]
        def __eq__(self, other: typing.Any) -> bool: ...
        def __getstate__(self) -> int: ...
        def __hash__(self) -> int: ...
        def __index__(self) -> int: ...
        def __init__(self, value: typing.SupportsInt) -> None: ...
        def __int__(self) -> int: ...
        def __ne__(self, other: typing.Any) -> bool: ...
        def __repr__(self) -> str: ...
        def __setstate__(self, state: typing.SupportsInt) -> None: ...
        def __str__(self) -> str: ...
        @property
        def name(self) -> str: ...
        @property
        def value(self) -> int: ...

    FT_BOOLEAN: typing.ClassVar[Layer.AttributeType]
    FT_DATE: typing.ClassVar[Layer.AttributeType]
    FT_DATETIME: typing.ClassVar[Layer.AttributeType]
    FT_INTEGER: typing.ClassVar[Layer.AttributeType]
    FT_INTEGER64: typing.ClassVar[Layer.AttributeType]
    FT_JSON: typing.ClassVar[Layer.AttributeType]
    FT_REAL: typing.ClassVar[Layer.AttributeType]
    FT_STRING: typing.ClassVar[Layer.AttributeType]
    FT_TIME: typing.ClassVar[Layer.AttributeType]
    GT_LINESTRING: typing.ClassVar[Layer.GeometryType]
    GT_LINESTRINGZ: typing.ClassVar[Layer.GeometryType]
    GT_MULTILINESTRING: typing.ClassVar[Layer.GeometryType]
    GT_MULTILINESTRINGZ: typing.ClassVar[Layer.GeometryType]
    GT_MULTIPOINT: typing.ClassVar[Layer.GeometryType]
    GT_MULTIPOINTZ: typing.ClassVar[Layer.GeometryType]
    GT_MULTIPOLYGON: typing.ClassVar[Layer.GeometryType]
    GT_MULTIPOLYGONZ: typing.ClassVar[
        Layer.GeometryType
    ]  # value = <GeometryType.GT_MULTIPOLYGONZ: 11>
    GT_NULL: typing.ClassVar[Layer.GeometryType]
    GT_POINT: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_POINT: 0>
    GT_POINTZ: typing.ClassVar[Layer.GeometryType]
    GT_POLYGON: typing.ClassVar[Layer.GeometryType]
    GT_POLYGONZ: typing.ClassVar[Layer.GeometryType]
    GT_UNKNOWN: typing.ClassVar[Layer.GeometryType]

    @staticmethod
    def from_data(
        geometry_type: Layer.GeometryType, crs: CRS, attribute_types: tuple, features: tuple
    ) -> Layer: ...
    @staticmethod
    def from_gdal(uri: typing.Any) -> Layer: ...
    @staticmethod
    def from_ogr(uri: typing.Any) -> Layer: ...

class LayerType:
    """
    Members:

      LT_VECTOR

      LT_RASTER

      LT_UNKNOWN
    """

    LT_RASTER: typing.ClassVar[LayerType]
    LT_UNKNOWN: typing.ClassVar[LayerType]
    LT_VECTOR: typing.ClassVar[LayerType]
    __members__: typing.ClassVar[dict[str, LayerType]]

    def __eq__(self, other: typing.Any) -> bool: ...
    def __getstate__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __init__(self, value: typing.SupportsInt) -> None: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: typing.Any) -> bool: ...
    def __repr__(self) -> str: ...
    def __setstate__(self, state: typing.SupportsInt) -> None: ...
    def __str__(self) -> str: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class LegendSymbol:
    def icon(self) -> Image: ...
    def index(self) -> int: ...
    def is_group(self) -> bool: ...
    def parent(self) -> int | None: ...
    def raster_band(self) -> int: ...
    def render(self) -> bool | None: ...
    def title(self) -> str | None: ...

class LogLevel:
    """
    Members:

      DEBUG

      INFO

      WARNING

      CRITICAL
    """

    CRITICAL: typing.ClassVar[LogLevel]
    DEBUG: typing.ClassVar[LogLevel]
    INFO: typing.ClassVar[LogLevel]
    WARNING: typing.ClassVar[LogLevel]
    __members__: typing.ClassVar[dict[str, LogLevel]]

    def __eq__(self, other: typing.Any) -> bool: ...
    def __getstate__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __init__(self, value: typing.SupportsInt) -> None: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: typing.Any) -> bool: ...
    def __repr__(self) -> str: ...
    def __setstate__(self, state: typing.SupportsInt) -> None: ...
    def __str__(self) -> str: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class MapRequest:
    def __init__(self) -> None: ...
    def add_layer(self, layer: Layer, style: Style, label: str = "") -> int: ...
    def add_project(self, project: Project) -> None: ...
    def export_pdf(
        self,
        filepath: str,
        extent: tuple[
            typing.SupportsFloat, typing.SupportsFloat, typing.SupportsFloat, typing.SupportsFloat
        ],
        size: tuple[typing.SupportsInt, typing.SupportsInt],
    ) -> None: ...
    def legend_symbols(
        self,
        index: typing.SupportsInt,
        size: tuple[typing.SupportsInt, typing.SupportsInt] = (0, 0),
        count: typing.SupportsInt = 5,
    ) -> list[LegendSymbol]: ...
    def render_image(
        self,
        extent: tuple[
            typing.SupportsFloat, typing.SupportsFloat, typing.SupportsFloat, typing.SupportsFloat
        ],
        size: tuple[typing.SupportsInt, typing.SupportsInt],
        *,
        symbols: tuple | None = None,
    ) -> Image: ...
    def render_legend(
        self, size: tuple[typing.SupportsInt, typing.SupportsInt] = (0, 0)
    ) -> Image: ...
    def set_crs(self, crs: CRS) -> None: ...
    def set_dpi(self, dpi: typing.SupportsInt) -> None: ...

class Project:
    @staticmethod
    def from_file(filename: typing.Any) -> Project: ...
    def __init__(self) -> None: ...

class QgisHeadlessError(Exception):
    pass

class RawData:
    def __init__(self) -> None: ...
    def size(self) -> int: ...
    def to_bytes(self) -> memoryview: ...

class Style:
    @staticmethod
    def from_defaults(
        color: tuple | None = None,
        layer_geometry_type: Layer.GeometryType = Layer.GeometryType.GT_UNKNOWN,
        layer_type: LayerType = LayerType.LT_UNKNOWN,
        random_seed: typing.SupportsInt = 0,
    ) -> Style: ...
    @staticmethod
    def from_file(
        file_path: typing.Any,
        svg_resolver: collections.abc.Callable[[str], str] = None,
        layer_geometry_type: Layer.GeometryType = Layer.GeometryType.GT_UNKNOWN,
        layer_type: LayerType = LayerType.LT_UNKNOWN,
        format: StyleFormat = StyleFormat.QML,
    ) -> Style: ...
    @staticmethod
    def from_string(
        string: str,
        svg_resolver: collections.abc.Callable[[str], str] = None,
        layer_geometry_type: Layer.GeometryType = Layer.GeometryType.GT_UNKNOWN,
        layer_type: LayerType = LayerType.LT_UNKNOWN,
        format: StyleFormat = StyleFormat.QML,
    ) -> Style: ...
    def scale_range(self) -> tuple: ...
    def to_string(self, format: StyleFormat = StyleFormat.QML) -> str: ...
    def used_attributes(self) -> set[str] | None: ...

class StyleFormat:
    """
    Members:

      QML

      SLD
    """

    QML: typing.ClassVar[StyleFormat]
    SLD: typing.ClassVar[StyleFormat]
    __members__: typing.ClassVar[dict[str, StyleFormat]]

    def __eq__(self, other: typing.Any) -> bool: ...
    def __getstate__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __init__(self, value: typing.SupportsInt) -> None: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: typing.Any) -> bool: ...
    def __repr__(self) -> str: ...
    def __setstate__(self, state: typing.SupportsInt) -> None: ...
    def __str__(self) -> str: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class StyleTypeMismatch(StyleValidationError):
    pass

class StyleValidationError(QgisHeadlessError):
    pass

def deinit() -> None:
    """
    Library deinitialization
    """

def get_qgis_version() -> str:
    """
    Get QGIS library version
    """

def get_qgis_version_int() -> int:
    """
    Get QGIS library version (number)
    """

def get_svg_paths() -> list[str]:
    """
    Get SVG search paths
    """

def get_version() -> str:
    """
    Get library version
    """

def init(args: collections.abc.Sequence[str]) -> None:
    """
    Library initialization
    """

def set_logging_level(level: LogLevel) -> None:
    """
    Set logging level
    """

def set_svg_paths(paths: collections.abc.Sequence[str]) -> None:
    """
    Set SVG search paths
    """

CRITICAL: LogLevel
DEBUG: LogLevel
INFO: LogLevel
LT_RASTER: LayerType
LT_UNKNOWN: LayerType
LT_VECTOR: LayerType
SF_QML: StyleFormat
SF_SLD: StyleFormat
WARNING: LogLevel
