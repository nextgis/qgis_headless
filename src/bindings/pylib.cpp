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

    pybind11::enum_<HeadlessRender::LayerGeometryType>( layer, "GeometryType" )
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

    pybind11::enum_<HeadlessRender::LayerAttributeType>( layer, "AttributeType" )
        .value("FT_INTEGER", HeadlessRender::LayerAttributeType::Integer)
        .value("FT_REAL", HeadlessRender::LayerAttributeType::Real)
        .value("FT_STRING", HeadlessRender::LayerAttributeType::String)
        .value("FT_DATE", HeadlessRender::LayerAttributeType::Date)
        .value("FT_TIME", HeadlessRender::LayerAttributeType::Time)
        .value("FT_DATETIME", HeadlessRender::LayerAttributeType::DateTime)
        .value("FT_INTEGER64", HeadlessRender::LayerAttributeType::Integer64)
        .export_values();

    pybind11::enum_<HeadlessRender::DataType>( m, "LayerType" )
        .value("LT_VECTOR", HeadlessRender::DataType::Vector)
        .value("LT_RASTER", HeadlessRender::DataType::Raster)
        .value("LT_UNKNOWN", HeadlessRender::DataType::Unknown)
        .export_values();

    layer.def( pybind11::init<>() )
        .def_static( "from_ogr", &HeadlessRender::Layer::fromOgr )
        .def_static( "from_gdal", &HeadlessRender::Layer::fromGdal )
        .def_static( "from_data", []( HeadlessRender::LayerGeometryType geometryType,
                               const HeadlessRender::CRS &crs,
                               const pybind11::tuple &attrTypes,
                               const pybind11::tuple &features )
        {

            QVector<QPair<QString, HeadlessRender::LayerAttributeType>> attributeTypes;
            QVector<HeadlessRender::Layer::FeatureData> featureData;

            for ( const auto &it : attrTypes )
            {
                const pybind11::tuple &attr = it.cast<pybind11::tuple>();
                attributeTypes.append( qMakePair( QString::fromStdString( attr[0].cast<std::string>() ), attr[1].cast<HeadlessRender::LayerAttributeType>() ) );
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
                    HeadlessRender::LayerAttributeType attrType = attributeTypes[idx++].second;

                    if ( attr.is_none() )
                    {
                        feature.attributes.append( QVariant( HeadlessRender::layerAttributeTypetoQVariantType( attrType ) ) );
                        continue;
                    }

                    switch( attrType )
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
                        const pybind11::tuple &params = attr.cast<pybind11::tuple>();
                        int y = params[0].cast<int>();
                        int m = params[1].cast<int>();
                        int d = params[2].cast<int>();
                        feature.attributes.append( QDate( y, m, d ) );
                        break;
                    }
                    case HeadlessRender::LayerAttributeType::Time:
                    {
                        const pybind11::tuple &params = attr.cast<pybind11::tuple>();
                        int h = params[0].cast<int>();
                        int m = params[1].cast<int>();
                        int s = params[2].cast<int>();
                        feature.attributes.append( QTime( h, m, s ) );
                        break;
                    }
                    case HeadlessRender::LayerAttributeType::DateTime:
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
                    case HeadlessRender::LayerAttributeType::Integer64:
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

    pybind11::enum_<HeadlessRender::StyleFormat>( m, "StyleFormat" )
        .value("QML", HeadlessRender::StyleFormat::QML)
        .value("SLD", HeadlessRender::StyleFormat::SLD)
        .export_values();

    pybind11::class_<HeadlessRender::Style>( m, "Style" )
        .def_static( "from_string", &HeadlessRender::Style::fromString,
                     pybind11::arg("string"),
                     pybind11::arg("svg_resolver") = nullptr,
                     pybind11::arg("layer_geometry_type") = HeadlessRender::LayerGeometryType::Unknown,
                     pybind11::arg("layer_type") = HeadlessRender::DataType::Unknown,
                     pybind11::arg("format") = HeadlessRender::StyleFormat::QML )
        .def_static( "from_file", &HeadlessRender::Style::fromFile,
                     pybind11::arg("filePath"),
                     pybind11::arg("svg_resolver") = nullptr,
                     pybind11::arg("layer_geometry_type") = HeadlessRender::LayerGeometryType::Unknown,
                     pybind11::arg("layer_type") = HeadlessRender::DataType::Unknown,
                     pybind11::arg("format") = HeadlessRender::StyleFormat::QML )
        .def( "used_attributes", []( const HeadlessRender::Style &style ) -> pybind11::object
        {
            const auto &result = style.usedAttributes();
            if ( result.first )
                return pybind11::cast( result.second );
            else
                return pybind11::none();
        })
        .def( "scale_range", []( const HeadlessRender::Style &style ) -> pybind11::tuple
        {
            const auto &result = style.scaleRange();
            return pybind11::make_tuple(
                ( result[0] > 0 ) ? pybind11::cast(result[0]) : pybind11::none(),
                ( result[1] > 0 ) ? pybind11::cast(result[1]) : pybind11::none()
            );
        })
        .def_static( "from_defaults", []( const pybind11::object &color,
                     HeadlessRender::LayerGeometryType layer_geometry_type,
                     HeadlessRender::DataType layer_type )
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
            return HeadlessRender::Style::fromDefaults( qcolor, layer_geometry_type, layer_type );
        }, pybind11::arg("color") = pybind11::none(), pybind11::arg("layer_geometry_type") = HeadlessRender::LayerGeometryType::Unknown, pybind11::arg("layer_type") = HeadlessRender::DataType::Unknown )
        .def( "to_string", []( const HeadlessRender::Style &style, const HeadlessRender::StyleFormat format  )
        {
            return style.exportToString( format ).toStdString();
        }, pybind11::arg("format") = HeadlessRender::StyleFormat::QML);

    pybind11::class_<HeadlessRender::LegendSymbol::RasterBand>(m, "RasterBand" )
            .def( "red", &HeadlessRender::LegendSymbol::RasterBand::red)
            .def( "green", &HeadlessRender::LegendSymbol::RasterBand::green)
            .def( "blue", &HeadlessRender::LegendSymbol::RasterBand::blue)
            .def( "alpha", &HeadlessRender::LegendSymbol::RasterBand::alpha);

    pybind11::class_<HeadlessRender::LegendSymbol>( m, "LegendSymbol" )
        .def( "icon", &HeadlessRender::LegendSymbol::icon )
        .def( "title", []( const HeadlessRender::LegendSymbol &legendSymbol ) -> pybind11::object
        {
            const QString title = legendSymbol.title();
            if ( !legendSymbol.hasCategory() && title.isEmpty() )
                return pybind11::none();
            else
                return pybind11::cast( title.toStdString() );
        })
        .def( "index", &HeadlessRender::LegendSymbol::index )
        .def( "render", &HeadlessRender::LegendSymbol::isEnabled )
        .def( "raster_band", &HeadlessRender::LegendSymbol::rasterBand );

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
        .def( "render_image", []( HeadlessRender::MapRequest &mapRequest, const HeadlessRender::Extent &extent, const HeadlessRender::Size &size, const pybind11::object &symbols )
        {
            if ( symbols.is_none() )
                return mapRequest.renderImage( extent, size );

            HeadlessRender::RenderSymbols renderSymbols;
            for ( const auto &it : symbols.cast<pybind11::tuple>() )
            {
                const auto layerRenderSymbols = it.cast<pybind11::tuple>();
                HeadlessRender::SymbolIndexVector symbolIndexVector;
                for ( const auto &symbolIt : layerRenderSymbols[1].cast<pybind11::tuple>())
                    symbolIndexVector.push_back( symbolIt.cast<HeadlessRender::LegendSymbol::Index>() );
                renderSymbols[layerRenderSymbols[0].cast<HeadlessRender::LayerIndex>()] = symbolIndexVector;
            }
            return mapRequest.renderImage( extent, size, renderSymbols );
        }, pybind11::arg("extent"), pybind11::arg("size"), pybind11::kw_only(), pybind11::arg("symbols") = pybind11::none())
        .def( "render_legend", &HeadlessRender::MapRequest::renderLegend, pybind11::arg("size") = HeadlessRender::Size() )
        .def( "export_pdf", &HeadlessRender::MapRequest::exportPdf )
        .def( "legend_symbols", &HeadlessRender::MapRequest::legendSymbols, pybind11::arg("index"), pybind11::arg("size") = HeadlessRender::Size(), pybind11::arg("count") = HeadlessRender::InvalidValue );

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
