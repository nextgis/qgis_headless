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

#include <lib.h>

PYBIND11_MODULE(_qgis_headless, m) {

    pybind11::class_<HeadlessRender::CRS>( m, "CRS" )
        .def( pybind11::init<>() )
        .def_static( "from_epsg", &HeadlessRender::CRS::fromEPSG );

    pybind11::class_<HeadlessRender::Style>( m, "Style" )
        .def( pybind11::init<>()  )
        .def_static( "from_string", &HeadlessRender::Style::fromString, pybind11::arg("string"), pybind11::arg("svg_resolver") = nullptr )
        .def_static( "from_file", &HeadlessRender::Style::fromFile, pybind11::arg("filePath"), pybind11::arg("svg_resolver") = nullptr );

    pybind11::class_<HeadlessRender::Layer> layer( m, "Layer" );

    layer.def( pybind11::init<>() )
        .def_static( "from_ogr", &HeadlessRender::Layer::fromOgr )
        .def_static( "from_gdal", &HeadlessRender::Layer::fromGdal )
        .def( "from_data", []( HeadlessRender::Layer::GeometryType geometryType,
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

                feature.id = feat[0].cast<long long int>();
                feature.wkb = feat[1].cast<std::string>();

                int idx = 0;
                for (const auto &attr : feat[2])
                {
                    HeadlessRender::Layer::AttributeType attrType = attributeTypes[idx++].second;
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
                    case HeadlessRender::Layer::AttributeType::Date: // fall through
                    case HeadlessRender::Layer::AttributeType::Time: // fall through
                    case HeadlessRender::Layer::AttributeType::DateTime:
                    {
                        qint64 msec = attr.cast<qint64>();
                        QDateTime dateTime;
                        dateTime.setSecsSinceEpoch( msec );

                        if ( attrType == HeadlessRender::Layer::AttributeType::Date )
                            feature.attributes.append( dateTime.date() );
                        else if ( attrType == HeadlessRender::Layer::AttributeType::Date )
                            feature.attributes.append( dateTime.time() );
                        else
                            feature.attributes.append( dateTime );

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

    pybind11::enum_<HeadlessRender::Layer::GeometryType>( layer, "GeometryType" )
        .value( "GT_POINT", HeadlessRender::Layer::GeometryType::Point )
        .value( "GT_LINESTRING", HeadlessRender::Layer::GeometryType::LineString )
        .value( "GT_POLYGON", HeadlessRender::Layer::GeometryType::Polygon )
        .value( "GT_MULTIPOINT", HeadlessRender::Layer::GeometryType::MultiPoint )
        .value( "GT_MULTILINESTRING", HeadlessRender::Layer::GeometryType::MultiLineString )
        .value( "GT_MULTIPOLYGON", HeadlessRender::Layer::GeometryType::MultiPolygon )
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


    pybind11::class_<HeadlessRender::Image, std::shared_ptr<HeadlessRender::Image>>( m, "Image" )
        .def( pybind11::init<>() )
        .def( "to_bytes", []( std::shared_ptr<HeadlessRender::Image> img ) {
            return pybind11::bytes( img->toString() );
        });

    pybind11::class_<HeadlessRender::MapRequest>( m, "MapRequest" )
        .def( pybind11::init<>() )
        .def( "set_dpi", &HeadlessRender::MapRequest::setDpi )
        .def( "set_crs", &HeadlessRender::MapRequest::setCrs )
        .def( "add_layer", &HeadlessRender::MapRequest::addLayer, pybind11::arg("layer"), pybind11::arg("style"), pybind11::arg("label") = "" )
        .def( "render_image", &HeadlessRender::MapRequest::renderImage )
        .def( "render_legend", &HeadlessRender::MapRequest::renderLegend, pybind11::arg("size") = HeadlessRender::Size() );

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

    m.def("get_version", &HeadlessRender::getVersion, "Get library version");
}
