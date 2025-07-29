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
        """

        FT_DATE: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_DATE: 3>
        FT_DATETIME: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_DATETIME: 5>
        FT_INTEGER: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_INTEGER: 0>
        FT_INTEGER64: typing.ClassVar[
            Layer.AttributeType
        ]  # value = <AttributeType.FT_INTEGER64: 6>
        FT_REAL: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_REAL: 1>
        FT_STRING: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_STRING: 2>
        FT_TIME: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_TIME: 4>
        __members__: typing.ClassVar[
            dict[str, Layer.AttributeType]
        ]  # value = {'FT_INTEGER': <AttributeType.FT_INTEGER: 0>, 'FT_REAL': <AttributeType.FT_REAL: 1>, 'FT_STRING': <AttributeType.FT_STRING: 2>, 'FT_DATE': <AttributeType.FT_DATE: 3>, 'FT_TIME': <AttributeType.FT_TIME: 4>, 'FT_DATETIME': <AttributeType.FT_DATETIME: 5>, 'FT_INTEGER64': <AttributeType.FT_INTEGER64: 6>}
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

          GT_UNKNOWN
        """

        GT_LINESTRING: typing.ClassVar[
            Layer.GeometryType
        ]  # value = <GeometryType.GT_LINESTRING: 1>
        GT_LINESTRINGZ: typing.ClassVar[
            Layer.GeometryType
        ]  # value = <GeometryType.GT_LINESTRINGZ: 7>
        GT_MULTILINESTRING: typing.ClassVar[
            Layer.GeometryType
        ]  # value = <GeometryType.GT_MULTILINESTRING: 4>
        GT_MULTILINESTRINGZ: typing.ClassVar[
            Layer.GeometryType
        ]  # value = <GeometryType.GT_MULTILINESTRINGZ: 10>
        GT_MULTIPOINT: typing.ClassVar[
            Layer.GeometryType
        ]  # value = <GeometryType.GT_MULTIPOINT: 3>
        GT_MULTIPOINTZ: typing.ClassVar[
            Layer.GeometryType
        ]  # value = <GeometryType.GT_MULTIPOINTZ: 9>
        GT_MULTIPOLYGON: typing.ClassVar[
            Layer.GeometryType
        ]  # value = <GeometryType.GT_MULTIPOLYGON: 5>
        GT_MULTIPOLYGONZ: typing.ClassVar[
            Layer.GeometryType
        ]  # value = <GeometryType.GT_MULTIPOLYGONZ: 11>
        GT_POINT: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_POINT: 0>
        GT_POINTZ: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_POINTZ: 6>
        GT_POLYGON: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_POLYGON: 2>
        GT_POLYGONZ: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_POLYGONZ: 8>
        GT_UNKNOWN: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_UNKNOWN: 12>
        __members__: typing.ClassVar[
            dict[str, Layer.GeometryType]
        ]  # value = {'GT_POINT': <GeometryType.GT_POINT: 0>, 'GT_LINESTRING': <GeometryType.GT_LINESTRING: 1>, 'GT_POLYGON': <GeometryType.GT_POLYGON: 2>, 'GT_MULTIPOINT': <GeometryType.GT_MULTIPOINT: 3>, 'GT_MULTILINESTRING': <GeometryType.GT_MULTILINESTRING: 4>, 'GT_MULTIPOLYGON': <GeometryType.GT_MULTIPOLYGON: 5>, 'GT_POINTZ': <GeometryType.GT_POINTZ: 6>, 'GT_LINESTRINGZ': <GeometryType.GT_LINESTRINGZ: 7>, 'GT_POLYGONZ': <GeometryType.GT_POLYGONZ: 8>, 'GT_MULTIPOINTZ': <GeometryType.GT_MULTIPOINTZ: 9>, 'GT_MULTILINESTRINGZ': <GeometryType.GT_MULTILINESTRINGZ: 10>, 'GT_MULTIPOLYGONZ': <GeometryType.GT_MULTIPOLYGONZ: 11>, 'GT_UNKNOWN': <GeometryType.GT_UNKNOWN: 12>}
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

    FT_DATE: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_DATE: 3>
    FT_DATETIME: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_DATETIME: 5>
    FT_INTEGER: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_INTEGER: 0>
    FT_INTEGER64: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_INTEGER64: 6>
    FT_REAL: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_REAL: 1>
    FT_STRING: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_STRING: 2>
    FT_TIME: typing.ClassVar[Layer.AttributeType]  # value = <AttributeType.FT_TIME: 4>
    GT_LINESTRING: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_LINESTRING: 1>
    GT_LINESTRINGZ: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_LINESTRINGZ: 7>
    GT_MULTILINESTRING: typing.ClassVar[
        Layer.GeometryType
    ]  # value = <GeometryType.GT_MULTILINESTRING: 4>
    GT_MULTILINESTRINGZ: typing.ClassVar[
        Layer.GeometryType
    ]  # value = <GeometryType.GT_MULTILINESTRINGZ: 10>
    GT_MULTIPOINT: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_MULTIPOINT: 3>
    GT_MULTIPOINTZ: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_MULTIPOINTZ: 9>
    GT_MULTIPOLYGON: typing.ClassVar[
        Layer.GeometryType
    ]  # value = <GeometryType.GT_MULTIPOLYGON: 5>
    GT_MULTIPOLYGONZ: typing.ClassVar[
        Layer.GeometryType
    ]  # value = <GeometryType.GT_MULTIPOLYGONZ: 11>
    GT_POINT: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_POINT: 0>
    GT_POINTZ: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_POINTZ: 6>
    GT_POLYGON: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_POLYGON: 2>
    GT_POLYGONZ: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_POLYGONZ: 8>
    GT_UNKNOWN: typing.ClassVar[Layer.GeometryType]  # value = <GeometryType.GT_UNKNOWN: 12>
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

    LT_RASTER: typing.ClassVar[LayerType]  # value = <LayerType.LT_RASTER: 0>
    LT_UNKNOWN: typing.ClassVar[LayerType]  # value = <LayerType.LT_UNKNOWN: 2>
    LT_VECTOR: typing.ClassVar[LayerType]  # value = <LayerType.LT_VECTOR: 1>
    __members__: typing.ClassVar[
        dict[str, LayerType]
    ]  # value = {'LT_VECTOR': <LayerType.LT_VECTOR: 1>, 'LT_RASTER': <LayerType.LT_RASTER: 0>, 'LT_UNKNOWN': <LayerType.LT_UNKNOWN: 2>}
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

    CRITICAL: typing.ClassVar[LogLevel]  # value = <LogLevel.CRITICAL: 3>
    DEBUG: typing.ClassVar[LogLevel]  # value = <LogLevel.DEBUG: 0>
    INFO: typing.ClassVar[LogLevel]  # value = <LogLevel.INFO: 1>
    WARNING: typing.ClassVar[LogLevel]  # value = <LogLevel.WARNING: 2>
    __members__: typing.ClassVar[
        dict[str, LogLevel]
    ]  # value = {'DEBUG': <LogLevel.DEBUG: 0>, 'INFO': <LogLevel.INFO: 1>, 'WARNING': <LogLevel.WARNING: 2>, 'CRITICAL': <LogLevel.CRITICAL: 3>}
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
    ) -> Style: ...
    @staticmethod
    def from_file(
        file_path: typing.Any,
        svg_resolver: collections.abc.Callable[[str], str] = None,
        layer_geometry_type: Layer.GeometryType = Layer.GeometryType.GT_UNKNOWN,
        layer_type: LayerType = LayerType.LT_UNKNOWN,
        format: StyleFormat = ...,
    ) -> Style: ...
    @staticmethod
    def from_string(
        string: str,
        svg_resolver: collections.abc.Callable[[str], str] = None,
        layer_geometry_type: Layer.GeometryType = Layer.GeometryType.GT_UNKNOWN,
        layer_type: LayerType = LayerType.LT_UNKNOWN,
        format: StyleFormat = ...,
    ) -> Style: ...
    def scale_range(self) -> tuple: ...
    def to_string(self, format: StyleFormat = ...) -> str: ...
    def used_attributes(self) -> set[str] | None: ...

class StyleFormat:
    """
    Members:

      QML

      SLD
    """

    QML: typing.ClassVar[StyleFormat]  # value = <StyleFormat.QML: 0>
    SLD: typing.ClassVar[StyleFormat]  # value = <StyleFormat.SLD: 1>
    __members__: typing.ClassVar[
        dict[str, StyleFormat]
    ]  # value = {'QML': <StyleFormat.QML: 0>, 'SLD': <StyleFormat.SLD: 1>}
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

CRITICAL: LogLevel  # value = <LogLevel.CRITICAL: 3>
DEBUG: LogLevel  # value = <LogLevel.DEBUG: 0>
INFO: LogLevel  # value = <LogLevel.INFO: 1>
LT_RASTER: LayerType  # value = <LayerType.LT_RASTER: 0>
LT_UNKNOWN: LayerType  # value = <LayerType.LT_UNKNOWN: 2>
LT_VECTOR: LayerType  # value = <LayerType.LT_VECTOR: 1>
SF_QML: StyleFormat  # value = <StyleFormat.QML: 0>
SF_SLD: StyleFormat  # value = <StyleFormat.SLD: 1>
WARNING: LogLevel  # value = <LogLevel.WARNING: 2>
