/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*  Author:  Denis Ilyin, denis.ilyin@nextgis.com
*******************************************************************************
*  Copyright (C) 2020 NextGIS, info@nextgis.ru
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>

#include <lib.h>
#include <exceptions.h>
#include <utils.h>
#include <layer_feature_p.h>

#include <QJsonDocument>
#include <QJsonObject>

// Undefining Qt macro slots for preventing collision with pybind11 declarations:
#ifdef slots
#undef slots
#endif

#include <datetime.h>
#include <limits>
#include <string_view>

namespace py = pybind11;

namespace
{
  using HeadlessRender::LayerAttributeType;
  using AttributeTypes = QVector<QPair<QString, LayerAttributeType>>;

  constexpr int MaxJsonDepth = 64;
  constexpr int MicrosecondsPerMillisecond = 1000;

  template<typename T> py::buffer_info readOnlyByteBuffer( const T &exporter )
  {
    return py::buffer_info(
      const_cast<uchar *>( exporter.data() ), 1, "B", 1,
      { static_cast<py::ssize_t>( exporter.size() ) }, { 1 }, true
    );
  }

  py::tuple componentTuple( py::handle value, py::size_t minimumSize, const char *errorMessage )
  {
    auto components = value.cast<py::tuple>();
    if ( components.size() < minimumSize )
      throw py::value_error( errorMessage );
    return components;
  }

  void checkJsonDepth( int depth )
  {
    if ( depth >= MaxJsonDepth )
      throw py::value_error( "JSON nesting exceeds 64 levels" );
  }

  QString utf8String( py::handle value )
  {
    // The UTF-8 view is valid while value is alive. QString owns the decoded text.
    const auto text = value.cast<std::string_view>();
    if ( text.size() > static_cast<size_t>( std::numeric_limits<int>::max() ) )
      throw py::value_error( "Text exceeds the Qt string size limit" );
    return QString::fromUtf8( text.data(), static_cast<int>( text.size() ) );
  }

  QVariant jsonValue( py::handle value, int depth );

  QVariantMap jsonObject( const py::dict &object, int depth )
  {
    checkJsonDepth( depth );
    QVariantMap result;
    for ( auto item : object )
    {
      if ( !py::isinstance<py::str>( item.first ) )
        throw py::type_error( "JSON object keys must be strings" );
      const auto key = utf8String( item.first );
      result.insert( key, jsonValue( item.second, depth + 1 ) );
    }
    return result;
  }

  QVariantList jsonArray( const py::list &array, int depth )
  {
    checkJsonDepth( depth );
    QVariantList result;
    for ( auto item : array )
      result.append( jsonValue( item, depth + 1 ) );
    return result;
  }

  QVariant jsonValue( py::handle value, int depth )
  {
    if ( value.is_none() )
      return QVariant();
    if ( py::isinstance<py::bool_>( value ) )
      return value.cast<bool>();
    if ( py::isinstance<py::int_>( value ) )
      return QVariant::fromValue( value.cast<qint64>() );
    if ( py::isinstance<py::float_>( value ) )
      return value.cast<double>();
    if ( py::isinstance<py::str>( value ) )
      return utf8String( value );
    if ( py::isinstance<py::dict>( value ) )
      return jsonObject( py::reinterpret_borrow<py::dict>( value ), depth );
    if ( py::isinstance<py::list>( value ) )
      return jsonArray( py::reinterpret_borrow<py::list>( value ), depth );
    throw py::type_error( "Unsupported JSON value" );
  }

  QDate dateValue( py::handle value )
  {
    if ( PyDate_Check( value.ptr() ) && !PyDateTime_Check( value.ptr() ) )
      return QDate(
        PyDateTime_GET_YEAR( value.ptr() ), PyDateTime_GET_MONTH( value.ptr() ),
        PyDateTime_GET_DAY( value.ptr() )
      );

    const auto params = componentTuple( value, 3, "Date requires year, month and day" );
    return QDate( params[0].cast<int>(), params[1].cast<int>(), params[2].cast<int>() );
  }

  QTime timeValue( py::handle value )
  {
    if ( PyTime_Check( value.ptr() ) )
    {
      if ( !value.attr( "tzinfo" ).is_none()
           || PyDateTime_TIME_GET_MICROSECOND( value.ptr() ) % MicrosecondsPerMillisecond )
        throw py::value_error( "Time requires no timezone and millisecond precision" );
      return QTime(
        PyDateTime_TIME_GET_HOUR( value.ptr() ), PyDateTime_TIME_GET_MINUTE( value.ptr() ),
        PyDateTime_TIME_GET_SECOND( value.ptr() ),
        PyDateTime_TIME_GET_MICROSECOND( value.ptr() ) / MicrosecondsPerMillisecond
      );
    }

    const auto params = componentTuple( value, 3, "Time requires hour, minute and second" );
    return QTime( params[0].cast<int>(), params[1].cast<int>(), params[2].cast<int>() );
  }

  QDateTime dateTimeValue( py::handle value )
  {
    if ( PyDateTime_Check( value.ptr() ) )
    {
      if ( !value.attr( "tzinfo" ).is_none()
           || PyDateTime_DATE_GET_MICROSECOND( value.ptr() ) % MicrosecondsPerMillisecond )
        throw py::value_error( "Datetime requires no timezone and millisecond precision" );
      return QDateTime(
        QDate(
          PyDateTime_GET_YEAR( value.ptr() ), PyDateTime_GET_MONTH( value.ptr() ),
          PyDateTime_GET_DAY( value.ptr() )
        ),
        QTime(
          PyDateTime_DATE_GET_HOUR( value.ptr() ), PyDateTime_DATE_GET_MINUTE( value.ptr() ),
          PyDateTime_DATE_GET_SECOND( value.ptr() ),
          PyDateTime_DATE_GET_MICROSECOND( value.ptr() ) / MicrosecondsPerMillisecond
        )
      );
    }

    const auto params = componentTuple( value, 6, "Datetime requires six calendar components" );
    QDateTime result;
    result.setDate( QDate( params[0].cast<int>(), params[1].cast<int>(), params[2].cast<int>() ) );
    result.setTime( QTime( params[3].cast<int>(), params[4].cast<int>(), params[5].cast<int>() ) );
    return result;
  }

  QVariant jsonAttribute( py::handle value )
  {
    if ( py::isinstance<py::dict>( value ) )
      return jsonObject( py::reinterpret_borrow<py::dict>( value ), 0 );
    const QString json = utf8String( value );
    const QJsonDocument document = QJsonDocument::fromJson( json.toUtf8() );
    if ( document.isObject() )
      return document.object().toVariantMap();
    // Legacy JSON text that does not represent an object is a typed null.
    return QVariant( HeadlessRender::layerAttributeTypetoQVariantType( LayerAttributeType::JSON ) );
  }

  QVariant attributeValue( py::handle value, LayerAttributeType type )
  {
    if ( value.is_none() )
      return QVariant( HeadlessRender::layerAttributeTypetoQVariantType( type ) );

    switch ( type )
    {
      case LayerAttributeType::Integer:
        return value.cast<int>();
      case LayerAttributeType::Real:
        return value.cast<double>();
      case LayerAttributeType::String:
        return utf8String( value );
      case LayerAttributeType::Date:
        return dateValue( value );
      case LayerAttributeType::Time:
        return timeValue( value );
      case LayerAttributeType::DateTime:
        return dateTimeValue( value );
      case LayerAttributeType::Integer64:
        return QVariant::fromValue( value.cast<qint64>() );
      case LayerAttributeType::Boolean:
        return value.cast<bool>();
      case LayerAttributeType::JSON:
        return jsonAttribute( value );
    }
    throw py::type_error( "Unsupported attribute type" );
  }

  QByteArray borrowFeatureWkb( py::handle value )
  {
    if ( !PyBytes_Check( value.ptr() ) )
      return QByteArray::fromStdString( value.cast<std::string>() );
    const auto size = PyBytes_GET_SIZE( value.ptr() );
    if ( size > std::numeric_limits<int>::max() )
      throw py::value_error( "WKB exceeds the Qt byte array size limit" );
    return QByteArray::fromRawData( PyBytes_AS_STRING( value.ptr() ), static_cast<int>( size ) );
  }

  QgsAttributes featureAttributes( py::handle values, const AttributeTypes &attributeTypes )
  {
    const char *const countError = "Feature attribute count must match the number of fields";
    QgsAttributes attributes;
    attributes.reserve( attributeTypes.size() );
    int index = 0;
    for ( const auto &value : values )
    {
      if ( index >= attributeTypes.size() )
        throw py::value_error( countError );
      attributes.append( attributeValue( value, attributeTypes[index++].second ) );
    }
    if ( index != attributeTypes.size() )
      throw py::value_error( countError );
    return attributes;
  }

  void addPythonFeature(
    QgsVectorLayer &layer, const QgsFields &fields, const AttributeTypes &attributeTypes,
    const py::tuple &feature
  )
  {
    if ( feature.size() != 3 )
      throw py::value_error( "Feature must contain an id, WKB and attribute values" );

    const qint64 id = feature[0].cast<qint64>();
    // The feature tuple owns the bytes until native insertion finishes parsing WKB.
    const QByteArray wkb = borrowFeatureWkb( feature[1].ptr() );
    const QgsAttributes attributes = featureAttributes( feature[2].ptr(), attributeTypes );
    HeadlessRender::detail::addMemoryLayerFeature( layer, fields, id, wkb, attributes );
  }

  HeadlessRender::Layer layerFromPythonData(
    HeadlessRender::LayerGeometryType geometryType, const HeadlessRender::CRS &crs,
    const py::tuple &fields, const py::iterable &features
  )
  {
    AttributeTypes attributeTypes;
    for ( const auto &field : fields )
    {
      const auto definition = componentTuple( field, 2, "Field requires a name and type" );
      attributeTypes.append(
        { QString::fromStdString( definition[0].cast<std::string>() ),
          definition[1].cast<LayerAttributeType>() }
      );
    }

    auto layer = HeadlessRender::Layer::fromData( geometryType, crs, attributeTypes, {} );
    auto vectorLayer = std::static_pointer_cast<QgsVectorLayer>( layer.qgsMapLayer() );
    const QgsFields layerFields = vectorLayer->fields();
    for ( const auto &feature : features )
      addPythonFeature( *vectorLayer, layerFields, attributeTypes, feature.cast<py::tuple>() );
    return layer;
  }
} //namespace

PYBIND11_MODULE( _qgis_headless, m )
{
  PyDateTime_IMPORT;
  if ( !PyDateTimeAPI )
    throw py::error_already_set();
  py::enum_<HeadlessRender::LogLevel>( m, "LogLevel" )
    .value( "DEBUG", HeadlessRender::LogLevel::Debug )
    .value( "INFO", HeadlessRender::LogLevel::Info )
    .value( "WARNING", HeadlessRender::LogLevel::Warning )
    .value( "CRITICAL", HeadlessRender::LogLevel::Critical )
    .export_values();

  auto qgisHeadlessErrorHandle
    = py::register_exception<HeadlessRender::QgisHeadlessError>( m, "QgisHeadlessError" );
  auto styleValidationErrorHandle = py::register_exception<
    HeadlessRender::StyleValidationError>( m, "StyleValidationError", qgisHeadlessErrorHandle );
  py::register_exception<
    HeadlessRender::StyleTypeMismatch>( m, "StyleTypeMismatch", styleValidationErrorHandle );
  py::register_exception<
    HeadlessRender::InvalidLayerSource>( m, "InvalidLayerSource", qgisHeadlessErrorHandle );
  py::register_exception<HeadlessRender::InvalidCRSError>( m, "InvalidCRSError", qgisHeadlessErrorHandle );

  py::class_<HeadlessRender::CRS>( m, "CRS" )
    .def( py::init<>() )
    .def_static( "from_epsg", &HeadlessRender::CRS::fromEPSG, py::arg( "epsg" ) )
    .def_static( "from_wkt", &HeadlessRender::CRS::fromWkt, py::arg( "wkt" ) );

  py::class_<HeadlessRender::Layer> layer( m, "Layer" );

  py::enum_<HeadlessRender::LayerGeometryType>( layer, "GeometryType" )
    .value( "GT_POINT", HeadlessRender::LayerGeometryType::Point )
    .value( "GT_LINESTRING", HeadlessRender::LayerGeometryType::LineString )
    .value( "GT_POLYGON", HeadlessRender::LayerGeometryType::Polygon )
    .value( "GT_MULTIPOINT", HeadlessRender::LayerGeometryType::MultiPoint )
    .value( "GT_MULTILINESTRING", HeadlessRender::LayerGeometryType::MultiLineString )
    .value( "GT_MULTIPOLYGON", HeadlessRender::LayerGeometryType::MultiPolygon )
    .value( "GT_POINTZ", HeadlessRender::LayerGeometryType::PointZ )
    .value( "GT_LINESTRINGZ", HeadlessRender::LayerGeometryType::LineStringZ )
    .value( "GT_POLYGONZ", HeadlessRender::LayerGeometryType::PolygonZ )
    .value( "GT_MULTIPOINTZ", HeadlessRender::LayerGeometryType::MultiPointZ )
    .value( "GT_MULTILINESTRINGZ", HeadlessRender::LayerGeometryType::MultiLineStringZ )
    .value( "GT_MULTIPOLYGONZ", HeadlessRender::LayerGeometryType::MultiPolygonZ )
    .value( "GT_NULL", HeadlessRender::LayerGeometryType::Null )
    .value( "GT_UNKNOWN", HeadlessRender::LayerGeometryType::Unknown )
    .export_values();

  py::enum_<HeadlessRender::LayerAttributeType>( layer, "AttributeType" )
    .value( "FT_INTEGER", HeadlessRender::LayerAttributeType::Integer )
    .value( "FT_REAL", HeadlessRender::LayerAttributeType::Real )
    .value( "FT_STRING", HeadlessRender::LayerAttributeType::String )
    .value( "FT_DATE", HeadlessRender::LayerAttributeType::Date )
    .value( "FT_TIME", HeadlessRender::LayerAttributeType::Time )
    .value( "FT_DATETIME", HeadlessRender::LayerAttributeType::DateTime )
    .value( "FT_INTEGER64", HeadlessRender::LayerAttributeType::Integer64 )
    .value( "FT_BOOLEAN", HeadlessRender::LayerAttributeType::Boolean )
    .value( "FT_JSON", HeadlessRender::LayerAttributeType::JSON )
    .export_values();

  py::enum_<HeadlessRender::DataType>( m, "LayerType" )
    .value( "LT_VECTOR", HeadlessRender::DataType::Vector )
    .value( "LT_RASTER", HeadlessRender::DataType::Raster )
    .value( "LT_UNKNOWN", HeadlessRender::DataType::Unknown )
    .export_values();

  layer
    .def_static(
      "from_ogr",
      []( const py::object &uri ) { return HeadlessRender::Layer::fromOgr( py::str( uri ) ); },
      py::arg( "uri" )
    )
    .def_static(
      "from_gdal",
      []( const py::object &uri ) { return HeadlessRender::Layer::fromGdal( py::str( uri ) ); },
      py::arg( "uri" )
    )
    .def_static( "from_data", layerFromPythonData, py::arg( "geometry_type" ), py::arg( "crs" ), py::arg( "attribute_types" ), py::arg( "features" ) );

  py::class_<HeadlessRender::Image, std::shared_ptr<HeadlessRender::Image>>( m, "Image", py::buffer_protocol() )
    .def_buffer( readOnlyByteBuffer<HeadlessRender::Image> )
    .def( "size", &HeadlessRender::Image::sizeWidthHeight )
    .def( "to_bytes", []( py::object image ) { return py::memoryview( image ); } );

  py::enum_<HeadlessRender::StyleFormat>( m, "StyleFormat" )
    .value( "QML", HeadlessRender::StyleFormat::QML )
    .value( "SLD", HeadlessRender::StyleFormat::SLD );
  m.attr( "SF_QML" ) = HeadlessRender::StyleFormat::QML;
  m.attr( "SF_SLD" ) = HeadlessRender::StyleFormat::SLD;

  py::class_<HeadlessRender::LayerSchema>( m, "LayerSchema" )
    .def_static( "vector", &HeadlessRender::LayerSchema::vector, py::arg( "crs" ), py::arg( "geometry_type" ), py::arg( "fields" ) )
    .def_static( "raster", &HeadlessRender::LayerSchema::raster, py::arg( "crs" ), py::arg( "band_types" ), py::arg( "width" ), py::arg( "height" ) );

  py::class_<HeadlessRender::Style>( m, "Style" )
    .def_static(
      "from_string", &HeadlessRender::Style::fromString, py::arg( "string" ),
      py::arg( "svg_resolver" ) = nullptr,
      py::arg( "layer_geometry_type" ) = HeadlessRender::LayerGeometryType::Unknown,
      py::arg( "layer_type" ) = HeadlessRender::DataType::Unknown,
      py::arg( "format" ) = HeadlessRender::StyleFormat::QML, py::kw_only(),
      py::arg( "layer_schema" ) = py::none()
    )
    .def_static(
      "from_file",
      [](
        const py::object &filePath, const HeadlessRender::SvgResolverCallback &svgResolverCallback,
        HeadlessRender::LayerGeometryType layerGeometryType, HeadlessRender::DataType layerType,
        HeadlessRender::StyleFormat format, const std::optional<HeadlessRender::LayerSchema> &layerSchema
      ) {
        return HeadlessRender::Style::
          fromFile( py::str( filePath ), svgResolverCallback, layerGeometryType, layerType, format, layerSchema );
      },
      py::arg( "file_path" ), py::arg( "svg_resolver" ) = nullptr,
      py::arg( "layer_geometry_type" ) = HeadlessRender::LayerGeometryType::Unknown,
      py::arg( "layer_type" ) = HeadlessRender::DataType::Unknown,
      py::arg( "format" ) = HeadlessRender::StyleFormat::QML, py::kw_only(),
      py::arg( "layer_schema" ) = py::none()
    )
    .def(
      "used_attributes",
      []( const HeadlessRender::Style &style ) -> std::optional<std::set<std::string>> {
        const auto &result = style.usedAttributes();
        if ( result.first )
          return result.second;
        else
          return std::optional<std::set<std::string>>();
      }
    )
    .def(
      "scale_range",
      []( const HeadlessRender::Style &style ) -> py::tuple {
        const auto &result = style.scaleRange();
        return py::
          make_tuple( ( result[0] > 0 ) ? py::cast( result[0] ) : py::none(), ( result[1] > 0 ) ? py::cast( result[1] ) : py::none() );
      }
    )
    .def_static(
      "from_defaults",
      [](
        const std::optional<py::tuple> &color,
        HeadlessRender::LayerGeometryType layer_geometry_type, HeadlessRender::DataType layer_type,
        HeadlessRender::RandomColorGenerator::SeedType random_seed
      ) {
        QColor qcolor;
        if ( color.has_value() )
        {
          const py::tuple &colorTuple = color.value();
          int r = colorTuple[0].cast<int>();
          int g = colorTuple[1].cast<int>();
          int b = colorTuple[2].cast<int>();
          int a = colorTuple[3].cast<int>();

          qcolor = { r, g, b, a };
        }
        return HeadlessRender::Style::fromDefaults( qcolor, layer_geometry_type, layer_type, random_seed );
      },
      py::arg( "color" ) = py::none(),
      py::arg( "layer_geometry_type" ) = HeadlessRender::LayerGeometryType::Unknown,
      py::arg( "layer_type" ) = HeadlessRender::DataType::Unknown, py::arg( "random_seed" ) = 0
    )
    .def(
      "to_string",
      []( const HeadlessRender::Style &style, const HeadlessRender::StyleFormat format ) {
        return style.exportToString( format ).toStdString();
      },
      py::arg( "format" ) = HeadlessRender::StyleFormat::QML
    );

  py::class_<HeadlessRender::LegendSymbol>( m, "LegendSymbol" )
    .def( "icon", &HeadlessRender::LegendSymbol::icon )
    .def(
      "title",
      []( const HeadlessRender::LegendSymbol &legendSymbol ) -> std::optional<py::str> {
        const QString title = legendSymbol.title();
        if ( !legendSymbol.hasTitle() || !legendSymbol.hasCategory() && title.isEmpty() )
          return std::optional<py::str>();
        else
          return py::cast( title.toStdString() );
      }
    )
    .def( "index", &HeadlessRender::LegendSymbol::index )
    .def(
      "render",
      []( const HeadlessRender::LegendSymbol &legendSymbol ) -> std::optional<bool> {
        switch ( legendSymbol.render() )
        {
          case HeadlessRender::SymbolRender::Checked:
            return true;
          case HeadlessRender::SymbolRender::Unchecked:
            return false;
          default:
            break;
        }
        return {};
      }
    )
    .def( "is_group", &HeadlessRender::LegendSymbol::isGroup )
    .def( "parent", &HeadlessRender::LegendSymbol::parent )
    .def( "raster_band", &HeadlessRender::LegendSymbol::rasterBand );

  py::class_<
    HeadlessRender::RawData,
    std::shared_ptr<HeadlessRender::RawData>>( m, "RawData", py::buffer_protocol() )
    .def_buffer( readOnlyByteBuffer<HeadlessRender::RawData> )
    .def( py::init<>() )
    .def( "size", &HeadlessRender::RawData::size )
    .def( "to_bytes", []( py::object data ) { return py::memoryview( data ); } );

  py::class_<HeadlessRender::Project>( m, "Project" )
    .def( py::init<>() )
    .def_static(
      "from_file",
      []( const py::object &filename ) {
        return HeadlessRender::Project::fromFile( py::str( filename ) );
      },
      py::arg( "filename" )
    );

  py::class_<HeadlessRender::MapRequest>( m, "MapRequest" )
    .def( py::init<>() )
    .def( "set_dpi", &HeadlessRender::MapRequest::setDpi, py::arg( "dpi" ) )
    .def( "set_crs", &HeadlessRender::MapRequest::setCrs, py::arg( "crs" ) )
    .def( "add_layer", &HeadlessRender::MapRequest::addLayer, py::arg( "layer" ), py::arg( "style" ), py::arg( "label" ) = "" )
    .def( "add_project", &HeadlessRender::MapRequest::addProject, py::arg( "project" ) )
    .def(
      "render_image",
      [](
        HeadlessRender::MapRequest &mapRequest, const HeadlessRender::Extent &extent,
        const HeadlessRender::Size &size, const std::optional<py::tuple> &symbols
      ) {
        if ( !symbols.has_value() )
          return mapRequest.renderImage( extent, size );

        HeadlessRender::RenderSymbols renderSymbols;
        for ( const auto &it : symbols.value() )
        {
          const auto layerRenderSymbols = it.cast<py::tuple>();
          HeadlessRender::SymbolIndexVector symbolIndexVector;
          for ( const auto &symbolIt : layerRenderSymbols[1].cast<py::tuple>() )
            symbolIndexVector.push_back( symbolIt.cast<HeadlessRender::LegendSymbol::Index>() );
          renderSymbols[layerRenderSymbols[0].cast<HeadlessRender::LayerIndex>()] = symbolIndexVector;
        }
        return mapRequest.renderImage( extent, size, renderSymbols );
      },
      py::arg( "extent" ), py::arg( "size" ), py::kw_only(), py::arg( "symbols" ) = py::none()
    )
    .def( "render_legend", &HeadlessRender::MapRequest::renderLegend, py::arg( "size" ) = HeadlessRender::Size() )
    .def( "export_pdf", &HeadlessRender::MapRequest::exportPdf, py::arg( "filepath" ), py::arg( "extent" ), py::arg( "size" ) )
    .def(
      "legend_symbols", &HeadlessRender::MapRequest::legendSymbols, py::arg( "index" ),
      py::arg( "size" ) = HeadlessRender::Size(),
      py::arg( "count" ) = HeadlessRender::DefaultRasterRenderSymbolCount
    );

  m.def(
    "init",
    []( const std::vector<std::string> &args ) {
      std::vector<char *> v;
      v.reserve( args.size() );
      for ( auto &s : args )
        v.push_back( const_cast<char *>( s.c_str() ) );
      return HeadlessRender::init( v.size(), v.data() );
    },
    "Library initialization", py::arg( "args" )
  );

  m.def( "deinit", &HeadlessRender::deinit, "Library deinitialization" );

  m.def( "set_svg_paths", &HeadlessRender::setSvgPaths, "Set SVG search paths", py::arg( "paths" ) );

  m.def( "get_svg_paths", &HeadlessRender::getSvgPaths, "Get SVG search paths" );

  m.def( "get_version", &HeadlessRender::getVersion, "Get library version" );

  m.def( "get_qgis_version", &HeadlessRender::getQGISVersion, "Get QGIS library version" );

  m.def( "get_qgis_version_int", &HeadlessRender::getQGISVersionInt, "Get QGIS library version (number)" );

  m.def( "set_logging_level", &HeadlessRender::setLoggingLevel, "Set logging level", py::arg( "level" ) );
}
