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

#include <pybind11.h>
#include <stl.h>
#include <functional.h>
#include <numpy.h>
#include <QDateTime>
#include <QColor>

#include <lib.h>
#include <exceptions.h>
#include <utils.h>

PYBIND11_MODULE(_qgis_headless, m) {

    pybind11::enum_<HeadlessRender::LogLevel>( m, "LogLevel" )
        .value("DEBUG", HeadlessRender::LogLevel::Debug)
        .value("INFO", HeadlessRender::LogLevel::Info)
        .value("WARNING", HeadlessRender::LogLevel::Warning)
        .value("CRITICAL", HeadlessRender::LogLevel::Critical)
        .export_values();

    auto qgisHeadlessErrorHandle = pybind11::register_exception<HeadlessRender::QgisHeadlessError>( m, "QgisHeadlessError" );
    auto styleValidationErrorHandle = pybind11::register_exception<HeadlessRender::StyleValidationError>( m, "StyleValidationError", qgisHeadlessErrorHandle );
    pybind11::register_exception<HeadlessRender::StyleTypeMismatch>( m, "StyleTypeMismatch", styleValidationErrorHandle );
    pybind11::register_exception<HeadlessRender::InvalidLayerSource>( m, "InvalidLayerSource", qgisHeadlessErrorHandle );

    pybind11::class_<HeadlessRender::Layer> layer( m, "Layer" );

    pybind11::enum_<HeadlessRender::Layer::GeometryType>( layer, "GeometryType" )
        .value( "GT_POINT", HeadlessRender::Layer::GeometryType::Point )
        .value( "GT_LINESTRING", HeadlessRender::Layer::GeometryType::LineString )
        .value( "GT_POLYGON", HeadlessRender::Layer::GeometryType::Polygon )
        .value( "GT_MULTIPOINT", HeadlessRender::Layer::GeometryType::MultiPoint )
        .value( "GT_MULTILINESTRING", HeadlessRender::Layer::GeometryType::MultiLineString )
        .value( "GT_MULTIPOLYGON", HeadlessRender::Layer::GeometryType::MultiPolygon )
        .value( "GT_POINTZ", HeadlessRender::Layer::GeometryType::PointZ )
        .value( "GT_LINESTRINGZ", HeadlessRender::Layer::GeometryType::LineStringZ )
        .value( "GT_POLYGONZ", HeadlessRender::Layer::GeometryType::PolygonZ )
        .value( "GT_MULTIPOINTZ", HeadlessRender::Layer::GeometryType::MultiPointZ )
        .value( "GT_MULTILINESTRINGZ", HeadlessRender::Layer::GeometryType::MultiLineStringZ )
        .value( "GT_MULTIPOLYGONZ", HeadlessRender::Layer::GeometryType::MultiPolygonZ )
        .value( "GT_UNKNOWN", HeadlessRender::Layer::GeometryType::Unknown )
        .export_values();

    pybind11::enum_<HeadlessRender::Layer::AttributeType>( layer, "AttributeType" )
        .value("FT_INTEGER", HeadlessRender::Layer::AttributeType::Integer)
        .value("FT_REAL", HeadlessRender::Layer::AttributeType::Real)
        .value("FT_STRING", HeadlessRender::Layer::AttributeType::String)
        .value("FT_DATE", HeadlessRender::Layer::AttributeType::Date)
        .value("FT_TIME", HeadlessRender::Layer::AttributeType::Time)
        .value("FT_DATETIME", HeadlessRender::Layer::AttributeType::DateTime)
        .value("FT_INTEGER64", HeadlessRender::Layer::AttributeType::Integer64)
        .export_values();

    pybind11::enum_<HeadlessRender::DataType>( m, "LayerType" )
        .value("LT_VECTOR", HeadlessRender::DataType::Vector)
        .value("LT_RASTER", HeadlessRender::DataType::Raster)
        .value("LT_UNKNOWN", HeadlessRender::DataType::Unknown)
        .export_values();

    layer.def( pybind11::init<>() )
        .def_static( "from_ogr", &HeadlessRender::Layer::fromOgr )
        .def_static( "from_gdal", &HeadlessRender::Layer::fromGdal )
        .def_static( "from_data", []( HeadlessRender::Layer::GeometryType geometryType,
                               const HeadlessRender::CRS &crs,
                               const pybind11::tuple &attrTypes,
                               const pybind11::tuple &features )
        {

            QVector<QPair<QString, HeadlessRender::Layer::AttributeType>> attributeTypes;
            QVector<HeadlessRender::Layer::FeatureData> featureData;

            for ( const auto &it : attrTypes )
            {
                const pybind11::tuple &attr = it.cast<pybind11::tuple>();
                attributeTypes.append( qMakePair( QString::fromStdString( attr[0].cast<std::string>() ), attr[1].cast<HeadlessRender::Layer::AttributeType>() ) );
            }

            for ( const auto &it : features )
            {
                HeadlessRender::Layer::FeatureData feature;

                const pybind11::tuple &feat = it.cast<pybind11::tuple>();

                feature.id = feat[0].cast<qint64>();
                feature.wkb = feat[1].cast<std::string>();
                

                int idx = 0;
                for (const auto &attr : feat[2])
                {
                    HeadlessRender::Layer::AttributeType attrType = attributeTypes[idx++].second;

                    if ( attr.is_none() )
                    {
                        feature.attributes.append( QVariant( HeadlessRender::layerAttributeTypetoQVariantType( attrType ) ) );
                        continue;
                    }

                    switch( attrType )
                    {
                    case HeadlessRender::Layer::AttributeType::Integer:
                        feature.attributes.append( attr.cast<int>() );
                        break;
                    case HeadlessRender::Layer::AttributeType::Real:
                        feature.attributes.append( attr.cast<double>() );
                        break;
                    case HeadlessRender::Layer::AttributeType::String:
                        feature.attributes.append( QString::fromStdString( attr.cast<std::string>() ) );
                        break;
                    case HeadlessRender::Layer::AttributeType::Date:
                    {
                        const pybind11::tuple &params = attr.cast<pybind11::tuple>();
                        int y = params[0].cast<int>();
                        int m = params[1].cast<int>();
                        int d = params[2].cast<int>();
                        feature.attributes.append( QDate( y, m, d ) );
                        break;
                    }
                    case HeadlessRender::Layer::AttributeType::Time:
                    {
                        const pybind11::tuple &params = attr.cast<pybind11::tuple>();
                        int h = params[0].cast<int>();
                        int m = params[1].cast<int>();
                        int s = params[2].cast<int>();
                        feature.attributes.append( QTime( h, m, s ) );
                        break;
                    }
                    case HeadlessRender::Layer::AttributeType::DateTime:
                    {
                        const pybind11::tuple &params = attr.cast<pybind11::tuple>();

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
                    case HeadlessRender::Layer::AttributeType::Integer64:
                        feature.attributes.append( attr.cast<qint64>() );
                        break;
                    }
                }

                featureData.append( feature );
            }

            return HeadlessRender::Layer::fromData( geometryType, crs, attributeTypes, featureData );
        });

    pybind11::class_<HeadlessRender::CRS>( m, "CRS" )
        .def( pybind11::init<>() )
        .def_static( "from_epsg", &HeadlessRender::CRS::fromEPSG )
        .def_static( "from_wkt", &HeadlessRender::CRS::fromWkt );

    pybind11::class_<HeadlessRender::Style>( m, "Style" )
        .def( pybind11::init<>()  )
        .def_static( "from_string", &HeadlessRender::Style::fromString, pybind11::arg("string"), pybind11::arg("svg_resolver") = nullptr,
                     pybind11::arg("layer_geometry_type") = HeadlessRender::Layer::GeometryType::Unknown, pybind11::arg("layer_type") = HeadlessRender::DataType::Unknown )
        .def_static( "from_file", &HeadlessRender::Style::fromFile, pybind11::arg("filePath"), pybind11::arg("svg_resolver") = nullptr,
                     pybind11::arg("layer_geometry_type") = HeadlessRender::Layer::GeometryType::Unknown, pybind11::arg("layer_type") = HeadlessRender::DataType::Unknown )
        .def( "used_attributes", []( const HeadlessRender::Style &style ) -> pybind11::object
        {
            const std::pair<bool, std::set<std::string>> &result = style.usedAttributes();
            if ( result.first )
                return pybind11::cast( result.second );
            else
                return pybind11::none();
        })
        .def_static( "from_defaults", []( const pybind11::object &color )
        {
            QColor qcolor;
            if ( !color.is_none() )
            {
                const pybind11::tuple &colorTuple = color.cast<pybind11::tuple>();
                int r = colorTuple[0].cast<int>();
                int g = colorTuple[1].cast<int>();
                int b = colorTuple[2].cast<int>();
                int a = colorTuple[3].cast<int>();

                qcolor = { r, g, b, a };
            }
            return HeadlessRender::Style::fromDefaults( qcolor );
        }, pybind11::arg("color") = pybind11::none() );

    pybind11::class_<HeadlessRender::LegendSymbol>( m, "LegendSymbol" )
        .def( "icon", &HeadlessRender::LegendSymbol::icon )
        .def( "title", []( const HeadlessRender::LegendSymbol &style )
        {
            return style.title().toStdString();
        });

    pybind11::class_<HeadlessRender::Image, std::shared_ptr<HeadlessRender::Image>>( m, "Image" )
        .def( pybind11::init<>() )
        .def( "size", &HeadlessRender::Image::sizeWidthHeight )
        .def( "to_bytes", []( std::shared_ptr<HeadlessRender::Image> img )
        {
            return pybind11::memoryview::from_memory( img->data(), img->size() );
        });

    pybind11::class_<HeadlessRender::RawData, std::shared_ptr<HeadlessRender::RawData>>( m, "RawData" )
        .def( pybind11::init<>() )
        .def( "size", &HeadlessRender::RawData::size )
        .def( "to_bytes", []( std::shared_ptr<HeadlessRender::RawData> bytes )
        {
            return pybind11::memoryview::from_memory( bytes->data(), bytes->size() );
        });

    pybind11::class_<HeadlessRender::Project>( m, "Project" )
        .def( pybind11::init<>() )
        .def_static( "from_file", &HeadlessRender::Project::fromFile );

    pybind11::class_<HeadlessRender::MapRequest>( m, "MapRequest" )
        .def( pybind11::init<>() )
        .def( "set_dpi", &HeadlessRender::MapRequest::setDpi )
        .def( "set_crs", &HeadlessRender::MapRequest::setCrs )
        .def( "add_layer", &HeadlessRender::MapRequest::addLayer, pybind11::arg("layer"), pybind11::arg("style"), pybind11::arg("label") = "" )
        .def( "add_project", &HeadlessRender::MapRequest::addProject )
        .def( "render_image", &HeadlessRender::MapRequest::renderImage )
        .def( "render_legend", &HeadlessRender::MapRequest::renderLegend, pybind11::arg("size") = HeadlessRender::Size() )
        .def( "export_pdf", &HeadlessRender::MapRequest::exportPdf )
        .def( "legend_symbols", &HeadlessRender::MapRequest::legendSymbols, pybind11::arg("index"), pybind11::arg("size") = HeadlessRender::Size() );

    m.def("init", []( const std::vector<std::string> &args )
    {
        std::vector<char *> v;
        v.reserve( args.size() );
        for ( auto &s : args )
            v.push_back( const_cast<char *>( s.c_str() ) );
        return HeadlessRender::init( v.size(), v.data() );
    }, "Library initialization");

    m.def("deinit", &HeadlessRender::deinit, "Library deinitialization");

    m.def("set_svg_paths", &HeadlessRender::setSvgPaths, "Set SVG search paths");

    m.def("get_svg_paths", &HeadlessRender::getSvgPaths, "Get SVG search paths");

    m.def("get_version", &HeadlessRender::getVersion, "Get library version");

    m.def("get_qgis_version", &HeadlessRender::getQGISVersion, "Get QGIS library version");

    m.def("set_logging_level", &HeadlessRender::setLoggingLevel, "Set logging level");
}
