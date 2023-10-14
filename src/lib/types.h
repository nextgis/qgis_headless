/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*  Author:  Denis Ilyin, denis.ilyin@nextgis.com
*******************************************************************************
*  Copyright (C) 2022 NextGIS, info@nextgis.ru
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

#ifndef QGIS_HEADLESS_TYPES_H
#define QGIS_HEADLESS_TYPES_H

#include <array>
#include <string>
#include <set>
#include <memory>

class QgsMapLayer;

namespace HeadlessRender
{
    enum class DataType
    {
        Raster,
        Vector,
        Unknown
    };

    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Critical
    };

    enum class LayerGeometryType
    {
        Point,
        LineString,
        Polygon,
        MultiPoint,
        MultiLineString,
        MultiPolygon,
        PointZ,
        LineStringZ,
        PolygonZ,
        MultiPointZ,
        MultiLineStringZ,
        MultiPolygonZ,
        Unknown
    };

    enum class LayerAttributeType
    {
        Integer,
        Real,
        String,
        Date,
        Time,
        DateTime,
        Integer64
    };

    enum class StyleFormat
    {
        QML,
        SLD
    };

    typedef std::pair<bool, std::set<std::string>> UsedAttributes;

    typedef std::array<double, 2> ScaleRange;

    typedef std::shared_ptr<QgsMapLayer> QgsMapLayerPtr;

    typedef long StyleCategory;
}

#endif // QGIS_HEADLESS_TYPES_H
