/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*  Author:  Denis Ilyin, denis.ilyin@nextgis.com
*******************************************************************************
*  Copyright (C) 2021 NextGIS, info@nextgis.ru
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

#include "utils.h"

QgsWkbTypes::Type HeadlessRender::layerGeometryTypeToQgsWkbType( HeadlessRender::Layer::GeometryType geometryType )
{
    switch( geometryType )
    {
    case HeadlessRender::Layer::GeometryType::Point:
        return QgsWkbTypes::Type::Point;
    case HeadlessRender::Layer::GeometryType::LineString:
        return QgsWkbTypes::Type::LineString;
    case HeadlessRender::Layer::GeometryType::Polygon:
        return QgsWkbTypes::Type::Polygon;
    case HeadlessRender::Layer::GeometryType::MultiPoint:
        return QgsWkbTypes::Type::MultiPoint;
    case HeadlessRender::Layer::GeometryType::MultiLineString:
        return QgsWkbTypes::Type::MultiLineString;
    case HeadlessRender::Layer::GeometryType::MultiPolygon:
        return QgsWkbTypes::Type::MultiPolygon;
    case HeadlessRender::Layer::GeometryType::PointZ:
        return QgsWkbTypes::Type::PointZ;
    case HeadlessRender::Layer::GeometryType::LineStringZ:
        return QgsWkbTypes::Type::LineStringZ;
    case HeadlessRender::Layer::GeometryType::PolygonZ:
        return QgsWkbTypes::Type::PolygonZ;
    case HeadlessRender::Layer::GeometryType::MultiPointZ:
        return QgsWkbTypes::Type::MultiPointZ;
    case HeadlessRender::Layer::GeometryType::MultiLineStringZ:
        return QgsWkbTypes::Type::MultiLineStringZ;
    case HeadlessRender::Layer::GeometryType::MultiPolygonZ:
        return QgsWkbTypes::Type::MultiPolygonZ;
    case HeadlessRender::Layer::GeometryType::Unknown:
        return QgsWkbTypes::Type::Unknown;
    }
}

QVariant::Type HeadlessRender::layerAttributeTypetoQVariantType( HeadlessRender::Layer::AttributeType attributeType )
{
    switch( attributeType )
    {
    case HeadlessRender::Layer::AttributeType::Integer:
        return QVariant::Int;
    case HeadlessRender::Layer::AttributeType::Real:
        return QVariant::Double;
    case HeadlessRender::Layer::AttributeType::String:
        return QVariant::String;
    case HeadlessRender::Layer::AttributeType::Date:
        return QVariant::Date;
    case HeadlessRender::Layer::AttributeType::Time:
        return QVariant::Time;
    case HeadlessRender::Layer::AttributeType::DateTime:
        return QVariant::DateTime;
    case HeadlessRender::Layer::AttributeType::Integer64:
        return QVariant::LongLong;
    }
}
