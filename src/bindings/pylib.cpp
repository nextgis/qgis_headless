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

// Undefining Qt macro slots for preventing collision with pybind11 declarations:
#ifdef slots
#undef slots
#endif

namespace py = pybind11;

PYBIND11_MODULE( _qgis_headless, m )
{
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
    .def_static(
      "from_data",
      []( HeadlessRender::LayerGeometryType geometryType, const HeadlessRender::CRS &crs, const py::tuple &attrTypes, const py::tuple &features ) {
        QVector<QPair<QString, HeadlessRender::LayerAttributeType>> attributeTypes;
        QVector<HeadlessRender::Layer::FeatureData> featureData;

        for ( const auto &it : attrTypes )
        {
          const py::tuple &attr = it.cast<py::tuple>();
          attributeTypes.append(
            qMakePair( QString::fromStdString( attr[0].cast<std::string>() ), attr[1].cast<HeadlessRender::LayerAttributeType>() )
          );
        }

        for ( const auto &it : features )
        {
          HeadlessRender::Layer::FeatureData feature;

          const py::tuple &feat = it.cast<py::tuple>();

          feature.id = feat[0].cast<qint64>();
          feature.wkb = feat[1].cast<std::string>();


          int idx = 0;
          for ( const auto &attr : feat[2] )
          {
            HeadlessRender::LayerAttributeType attrType = attributeTypes[idx++].second;

            if ( attr.is_none() )
            {
              feature.attributes.append(
                QVariant( HeadlessRender::layerAttributeTypetoQVariantType( attrType ) )
              );
              continue;
            }

            switch ( attrType )
            {
              case HeadlessRender::LayerAttributeType::Integer:
                feature.attributes.append( attr.cast<int>() );
                break;
              case HeadlessRender::LayerAttributeType::Real:
                feature.attributes.append( attr.cast<double>() );
                break;
              case HeadlessRender::LayerAttributeType::String:
                feature.attributes.append( QString::fromStdString( attr.cast<std::string>() ) );
                break;
              case HeadlessRender::LayerAttributeType::Date:
              {
                const py::tuple &params = attr.cast<py::tuple>();
                int y = params[0].cast<int>();
                int m = params[1].cast<int>();
                int d = params[2].cast<int>();
                feature.attributes.append( QDate( y, m, d ) );
                break;
              }
              case HeadlessRender::LayerAttributeType::Time:
              {
                const py::tuple &params = attr.cast<py::tuple>();
                int h = params[0].cast<int>();
                int m = params[1].cast<int>();
                int s = params[2].cast<int>();
                feature.attributes.append( QTime( h, m, s ) );
                break;
              }
              case HeadlessRender::LayerAttributeType::DateTime:
              {
                const py::tuple &params = attr.cast<py::tuple>();

                int year = params[0].cast<int>();
                int month = params[1].cast<int>();
                int day = params[2].cast<int>();
                int hour = params[3].cast<int>();
                int min = params[4].cast<int>();
                int sec = params[5].cast<int>();

                QDateTime datetime;
                datetime.setDate( QDate( year, month, day ) );
                datetime.setTime( QTime( hour, min, sec ) );

                feature.attributes.append( datetime );
                break;
              }
              case HeadlessRender::LayerAttributeType::Integer64:
                feature.attributes.append( attr.cast<qint64>() );
                break;
            }
          }

          featureData.append( feature );
        }

        return HeadlessRender::Layer::fromData( geometryType, crs, attributeTypes, featureData );
      },
      py::arg( "geometry_type" ), py::arg( "crs" ), py::arg( "attribute_types" ),
      py::arg( "features" )
    );

  py::class_<HeadlessRender::Image, std::shared_ptr<HeadlessRender::Image>>( m, "Image" )
    .def( "size", &HeadlessRender::Image::sizeWidthHeight )
    .def( "to_bytes", []( std::shared_ptr<HeadlessRender::Image> img ) {
      return py::memoryview::from_memory( img->data(), img->size() );
    } );

  py::enum_<HeadlessRender::StyleFormat>( m, "StyleFormat" )
    .value( "QML", HeadlessRender::StyleFormat::QML )
    .value( "SLD", HeadlessRender::StyleFormat::SLD );
  m.attr( "SF_QML" ) = HeadlessRender::StyleFormat::QML;
  m.attr( "SF_SLD" ) = HeadlessRender::StyleFormat::SLD;

  py::class_<HeadlessRender::Style>( m, "Style" )
    .def_static(
      "from_string", &HeadlessRender::Style::fromString, py::arg( "string" ),
      py::arg( "svg_resolver" ) = nullptr,
      py::arg( "layer_geometry_type" ) = HeadlessRender::LayerGeometryType::Unknown,
      py::arg( "layer_type" ) = HeadlessRender::DataType::Unknown,
      py::arg( "format" ) = HeadlessRender::StyleFormat::QML
    )
    .def_static(
      "from_file",
      [](
        const py::object &filePath, const HeadlessRender::SvgResolverCallback &svgResolverCallback,
        HeadlessRender::LayerGeometryType layerGeometryType, HeadlessRender::DataType layerType,
        HeadlessRender::StyleFormat format
      ) {
        return HeadlessRender::Style::
          fromFile( py::str( filePath ), svgResolverCallback, layerGeometryType, layerType, format );
      },
      py::arg( "file_path" ), py::arg( "svg_resolver" ) = nullptr,
      py::arg( "layer_geometry_type" ) = HeadlessRender::LayerGeometryType::Unknown,
      py::arg( "layer_type" ) = HeadlessRender::DataType::Unknown,
      py::arg( "format" ) = HeadlessRender::StyleFormat::QML
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
      []( const std::optional<py::tuple> &color, HeadlessRender::LayerGeometryType layer_geometry_type, HeadlessRender::DataType layer_type ) {
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
        return HeadlessRender::Style::fromDefaults( qcolor, layer_geometry_type, layer_type );
      },
      py::arg( "color" ) = py::none(),
      py::arg( "layer_geometry_type" ) = HeadlessRender::LayerGeometryType::Unknown,
      py::arg( "layer_type" ) = HeadlessRender::DataType::Unknown
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
      []( const HeadlessRender::LegendSymbol &legendSymbol ) -> std::optional<py::bool_> {
        switch ( legendSymbol.render() )
        {
          case HeadlessRender::SymbolRender::Checked:
            return py::cast( true );
          case HeadlessRender::SymbolRender::Unchecked:
            return py::cast( false );
          default:
            break;
        }
        return py::none();
      }
    )
    .def( "raster_band", &HeadlessRender::LegendSymbol::rasterBand );

  py::class_<HeadlessRender::RawData, std::shared_ptr<HeadlessRender::RawData>>( m, "RawData" )
    .def( py::init<>() )
    .def( "size", &HeadlessRender::RawData::size )
    .def( "to_bytes", []( std::shared_ptr<HeadlessRender::RawData> bytes ) {
      return py::memoryview::from_memory( bytes->data(), bytes->size() );
    } );

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
