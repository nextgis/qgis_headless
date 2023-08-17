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

QgsWkbTypes::Type HeadlessRender::layerGeometryTypeToQgsWkbType( HeadlessRender::LayerGeometryType geometryType )
{
    switch( geometryType )
    {
    case HeadlessRender::LayerGeometryType::Point:
        return QgsWkbTypes::Type::Point;
    case HeadlessRender::LayerGeometryType::LineString:
        return QgsWkbTypes::Type::LineString;
    case HeadlessRender::LayerGeometryType::Polygon:
        return QgsWkbTypes::Type::Polygon;
    case HeadlessRender::LayerGeometryType::MultiPoint:
        return QgsWkbTypes::Type::MultiPoint;
    case HeadlessRender::LayerGeometryType::MultiLineString:
        return QgsWkbTypes::Type::MultiLineString;
    case HeadlessRender::LayerGeometryType::MultiPolygon:
        return QgsWkbTypes::Type::MultiPolygon;
    case HeadlessRender::LayerGeometryType::PointZ:
        return QgsWkbTypes::Type::PointZ;
    case HeadlessRender::LayerGeometryType::LineStringZ:
        return QgsWkbTypes::Type::LineStringZ;
    case HeadlessRender::LayerGeometryType::PolygonZ:
        return QgsWkbTypes::Type::PolygonZ;
    case HeadlessRender::LayerGeometryType::MultiPointZ:
        return QgsWkbTypes::Type::MultiPointZ;
    case HeadlessRender::LayerGeometryType::MultiLineStringZ:
        return QgsWkbTypes::Type::MultiLineStringZ;
    case HeadlessRender::LayerGeometryType::MultiPolygonZ:
        return QgsWkbTypes::Type::MultiPolygonZ;
    case HeadlessRender::LayerGeometryType::Unknown:
        return QgsWkbTypes::Type::Unknown;
    }
}

QVariant::Type HeadlessRender::layerAttributeTypetoQVariantType( HeadlessRender::LayerAttributeType attributeType )
{
    switch( attributeType )
    {
    case HeadlessRender::LayerAttributeType::Integer:
        return QVariant::Int;
    case HeadlessRender::LayerAttributeType::Real:
        return QVariant::Double;
    case HeadlessRender::LayerAttributeType::String:
        return QVariant::String;
    case HeadlessRender::LayerAttributeType::Date:
        return QVariant::Date;
    case HeadlessRender::LayerAttributeType::Time:
        return QVariant::Time;
    case HeadlessRender::LayerAttributeType::DateTime:
        return QVariant::DateTime;
    case HeadlessRender::LayerAttributeType::Integer64:
        return QVariant::LongLong;
    }
}

HeadlessRender::QgsMapLayerPtr HeadlessRender::createTemporaryVectorLayer( const QgsVectorLayer::LayerOptions &layerOptions )
{
    return QgsMapLayerPtr( new QgsVectorLayer( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions ) );
}
