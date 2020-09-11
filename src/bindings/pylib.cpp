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

#include <lib.h>

PYBIND11_MODULE(_qgis_headless, m) {

    pybind11::class_<HeadlessRender::CRS>( m, "CRS" )
            .def( pybind11::init<>() )
            .def_static( "from_epsg", &HeadlessRender::CRS::fromEPSG );

    pybind11::class_<HeadlessRender::Style>( m, "Style" )
            .def( pybind11::init<>()  )
            .def_static( "from_string", &HeadlessRender::Style::fromString, pybind11::arg("string"), pybind11::arg("svgResolverCallback") = nullptr )
            .def_static( "from_file", &HeadlessRender::Style::fromFile, pybind11::arg("filePath"), pybind11::arg("svgResolverCallback") = nullptr );

    pybind11::class_<HeadlessRender::Layer>( m, "Layer" )
            .def( pybind11::init<>() )
            .def_static( "from_ogr", &HeadlessRender::Layer::fromOgr )
            .def_static( "from_gdal", &HeadlessRender::Layer::fromGdal );

    pybind11::class_<HeadlessRender::Image, std::shared_ptr<HeadlessRender::Image>>( m, "Image" )
            .def( pybind11::init<>() )
            .def( "to_bytes", []( std::shared_ptr<HeadlessRender::Image> img ) {
                return pybind11::bytes( img->toString() );
            });

    pybind11::class_<HeadlessRender::MapRequest>( m, "MapRequest" )
            .def( pybind11::init<>() )
            .def( "set_dpi", &HeadlessRender::MapRequest::setDpi )
            .def( "set_svg_paths", &HeadlessRender::MapRequest::setSvgPaths )
            .def( "set_crs", &HeadlessRender::MapRequest::setCrs )
            .def( "add_layer", &HeadlessRender::MapRequest::addLayer, pybind11::arg("layer"), pybind11::arg("style"), pybind11::arg("label") = "" )
            .def( "render_image", &HeadlessRender::MapRequest::renderImage )
            .def( "render_legend", &HeadlessRender::MapRequest::renderLegend, pybind11::arg("size") = HeadlessRender::Size() );

    m.def("init", []( const std::vector<std::string> &args ) {
        std::vector<char *> v;
        v.reserve( args.size() );
        for ( auto &s : args )
            v.push_back( const_cast<char *>( s.c_str() ) );
        return HeadlessRender::init( v.size(), v.data() );
    }, "Library initialization");

    m.def("deinit", &HeadlessRender::deinit, "Library deinitialization");

    m.def("get_version", &HeadlessRender::getVersion, "Get library version");
}
