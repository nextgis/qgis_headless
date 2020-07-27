/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*  Author:  Denis Ilyin, denis.ilyin@nextgis.com
*******************************************************************************
*  Copyright (C) 2020 NextGIS, info@nextgis.ru
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 2 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <lib.h>
#include <pybind11.h>
#include <stl.h>
#include <numpy.h>

PYBIND11_MODULE(qgis_headless_py, m) {

    pybind11::class_<HeadlessRender::Image, std::shared_ptr<HeadlessRender::Image>>(m, "Image")
            .def(pybind11::init<>())
            .def("size", &HeadlessRender::Image::getSize)
            .def("data",[](std::shared_ptr<HeadlessRender::Image> img) {
                return reinterpret_cast<uint64_t>(img->getData());
            });

    m.def("init", [](const std::vector<std::string> &args) {
        std::vector<char *> v;
        v.reserve(args.size());
        for (auto &s : args)
            v.push_back(const_cast<char *>(s.c_str()));
        return HeadlessRender::init(v.size(), v.data());
    }, "Library initialization");

    m.def("deinit", &HeadlessRender::deinit, "Library deinitialization");

    m.def("renderVector", &HeadlessRender::renderVector, "Render vector layer");

    m.def("rasterVector", &HeadlessRender::renderVector, "Render raster layer");

    m.def("getVersion", &HeadlessRender::getVersion, "Get library version");
}
