from enum import Enum
from typing import Any, Callable, Optional, Set, Tuple, Union

type Color = Tuple[int, int, int, int]
type Extent = Tuple[float, float, float, float]
type Size = Tuple[int, int]
type Paths = Tuple[str, ...]
type SvgResolverCallback = Callable[[str], str]
type LayerIndex = int
type SymbolIndex = int
type RenderSymbols = Tuple[Tuple[LayerIndex, Tuple[SymbolIndex, ...]], ...]

LogLevel = Enum(
    "LogLevel",
    [
        "DEBUG",
        "INFO",
        "WARNING",
        "CRIITCAL",
    ],
)
DEBUG = LogLevel.DEBUG
INFO = LogLevel.INFO
WARNING = LogLevel.WARNING
CRITICAL = LogLevel.CRIITCAL

StyleFormat = Enum(
    "StyleFormat",
    [
        "QML",
        "SLD",
    ],
)
QML = StyleFormat.QML
SLD = StyleFormat.SLD

LayerType = Enum(
    "LayerType",
    [
        "LT_VECTOR",
        "LT_RASTER",
        "LT_UNKNOWN",
    ],
)
LT_VECTOR = LayerType.LT_VECTOR
LT_RASTER = LayerType.LT_RASTER
LT_UNKNOWN = LayerType.LT_UNKNOWN

class QgisHeadlessError(Exception): ...
class StyleValidationError(QgisHeadlessError): ...
class StyleTypeMismatch(StyleValidationError): ...
class InvalidLayerSource(QgisHeadlessError): ...

class CRS:
    @classmethod
    def from_epsg(epsg: int) -> CRS: ...
    @classmethod
    def from_wkt(wkt: str) -> CRS: ...

class Layer:
    GeometryType = Enum(
        "GeometryType",
        [
            "GT_POINT",
            "GT_LINESTRING",
            "GT_POLYGON",
            "GT_MULTIPOINT",
            "GT_MULTILINESTRING",
            "GT_MULTIPOLYGON",
            "GT_POINTZ",
            "GT_LINESTRINGZ",
            "GT_POLYGONZ",
            "GT_MULTIPOINTZ",
            "GT_MULTILINESTRINGZ",
            "GT_MULTIPOLYGONZ",
            "GT_UNKNOWN",
        ],
    )
    GT_POINT = (GeometryType.GT_POINT,)
    GT_LINESTRING = GeometryType.GT_LINESTRING
    GT_POLYGON = GeometryType.GT_POLYGON
    GT_MULTIPOINT = GeometryType.GT_MULTIPOINT
    GT_MULTILINESTRING = GeometryType.GT_MULTILINESTRING
    GT_MULTIPOLYGON = GeometryType.GT_MULTIPOLYGON
    GT_POINTZ = GeometryType.GT_POINTZ
    GT_LINESTRINGZ = GeometryType.GT_LINESTRINGZ
    GT_POLYGONZ = GeometryType.GT_POLYGONZ
    GT_MULTIPOINTZ = GeometryType.GT_MULTIPOINTZ
    GT_MULTILINESTRINGZ = GeometryType.GT_MULTILINESTRINGZ
    GT_MULTIPOLYGONZ = GeometryType.GT_MULTIPOLYGONZ
    GT_UNKNOWN = GeometryType.GT_UNKNOWN

    AttributeType = Enum(
        "AttributeType",
        [
            "FT_INTEGER",
            "FT_REAL",
            "FT_STRING",
            "FT_DATE",
            "FT_TIME",
            "FT_DATETIME",
            "FT_INTEGER64",
        ],
    )
    FT_INTEGER = (AttributeType.FT_INTEGER,)
    FT_REAL = (AttributeType.FT_REAL,)
    FT_STRING = (AttributeType.FT_STRING,)
    FT_DATE = (AttributeType.FT_DATE,)
    FT_TIME = (AttributeType.FT_TIME,)
    FT_DATETIME = (AttributeType.FT_DATETIME,)
    FT_INTEGER64 = AttributeType.FT_INTEGER64

    @classmethod
    def from_ogr(uri: str) -> Layer: ...
    @classmethod
    def from_gdal(uri: str) -> Layer: ...
    @classmethod
    def from_data(
        geometryType: Layer.GeometryType,
        crs: CRS,
        attrTypes: Tuple[Tuple[str, Layer.AttributeType], ...],
        features: Tuple[Tuple[int, str, Tuple[Any, ...]], ...],
    ) -> Layer: ...

class Style:
    @classmethod
    def from_string(
        string: str,
        svg_resolver: SvgResolverCallback = None,
        layer_geometry_type: Layer.GeometryType = Layer.GeometryType.GT_UNKNOWN,
        layer_type: Layer.LayerType = Layer.LayerType.LT_UNKNOWN,
        format: StyleFormat = StyleFormat.QML,
    ) -> Style: ...
    @classmethod
    def from_file(
        filePath: str,
        svg_resolver: SvgResolverCallback = None,
        layer_geometry_type: Layer.GeometryType = Layer.GeometryType.GT_UNKNOWN,
        layer_type: Layer.LayerType = Layer.LayerType.LT_UNKNOWN,
        format: StyleFormat = StyleFormat.QML,
    ) -> Style: ...
    @classmethod
    def from_defaults(
        color: Color = None,
        layer_geometry_type: Layer.GeometryType = Layer.GeometryType.GT_UNKNOWN,
        layer_type: Layer.LayerType = Layer.LayerType.LT_UNKNOWN,
    ) -> Style: ...
    def to_string(format: StyleFormat = StyleFormat.QML) -> str: ...
    def used_attributes() -> Optional[Set[str]]: ...
    def scale_range() -> Union[Tuple[float, float], Tuple[None, None]]: ...

class Image:
    def size() -> Size: ...
    def to_bytes() -> bytes: ...

class LegendSymbol:
    def icon() -> Image: ...
    def title() -> str: ...
    def index() -> SymbolIndex: ...
    def render() -> Optional[bool]: ...

class MapRequest:
    def __init__() -> MapRequest: ...
    def set_dpi(dpi: int) -> None: ...
    def set_crs(crs: CRS) -> None: ...
    def add_layer(layer: Layer, style: Style, label: str = "") -> LayerIndex: ...
    def render_image(extent: Extent, size: Size, *, symbols: Optional[RenderSymbols] = None) -> Image: ...
    def render_legend(size: Size = [0, 0]) -> Image: ...
    def legend_symbols(index: LayerIndex, size: Size = (0, 0)) -> Tuple[LegendSymbol, ...]: ...

def init(args: Tuple[str, ...]) -> None: ...
def deinit() -> None: ...
def set_svg_paths(paths: Paths) -> None: ...
def get_svg_paths() -> Paths: ...
def get_version() -> str: ...
def get_qgis_version() -> str: ...
def set_logging_level(level: LogLevel) -> None: ...
