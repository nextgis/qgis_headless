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
#include <qgsrasterlayer.h>

#if _QGIS_VERSION_INT < 33000
QgsWkbTypes::Type HeadlessRender::layerGeometryTypeToQgsWkbType(
  HeadlessRender::LayerGeometryType geometryType
)
#else
Qgis::WkbType HeadlessRender::layerGeometryTypeToQgsWkbType( HeadlessRender::LayerGeometryType geometryType )
#endif
{
  switch ( geometryType )
  {
#if _QGIS_VERSION_INT < 33000
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
#else
    case HeadlessRender::LayerGeometryType::Point:
      return Qgis::WkbType::Point;
    case HeadlessRender::LayerGeometryType::LineString:
      return Qgis::WkbType::LineString;
    case HeadlessRender::LayerGeometryType::Polygon:
      return Qgis::WkbType::Polygon;
    case HeadlessRender::LayerGeometryType::MultiPoint:
      return Qgis::WkbType::MultiPoint;
    case HeadlessRender::LayerGeometryType::MultiLineString:
      return Qgis::WkbType::MultiLineString;
    case HeadlessRender::LayerGeometryType::MultiPolygon:
      return Qgis::WkbType::MultiPolygon;
    case HeadlessRender::LayerGeometryType::PointZ:
      return Qgis::WkbType::PointZ;
    case HeadlessRender::LayerGeometryType::LineStringZ:
      return Qgis::WkbType::LineStringZ;
    case HeadlessRender::LayerGeometryType::PolygonZ:
      return Qgis::WkbType::PolygonZ;
    case HeadlessRender::LayerGeometryType::MultiPointZ:
      return Qgis::WkbType::MultiPointZ;
    case HeadlessRender::LayerGeometryType::MultiLineStringZ:
      return Qgis::WkbType::MultiLineStringZ;
    case HeadlessRender::LayerGeometryType::MultiPolygonZ:
      return Qgis::WkbType::MultiPolygonZ;
    case HeadlessRender::LayerGeometryType::Unknown:
      return Qgis::WkbType::Unknown;
#endif
  }
#if _QGIS_VERSION_INT < 33000
  return QgsWkbTypes::Type::Unknown;
#else
  return Qgis::WkbType::Unknown;
#endif
}

QVariant::Type HeadlessRender::layerAttributeTypetoQVariantType(
  HeadlessRender::LayerAttributeType attributeType
)
{
  switch ( attributeType )
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
  return QVariant::Int;
}

HeadlessRender::QgsMapLayerPtr HeadlessRender::createTemporaryVectorLayer(
  const QgsVectorLayer::LayerOptions &layerOptions
)
{
  return std::make_shared<
    QgsVectorLayer>( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions );
}

HeadlessRender::QgsMapLayerPtr HeadlessRender::createTemporaryRasterLayer()
{
  QgsRasterLayer::LayerOptions layerOptions;
  layerOptions.loadDefaultStyle = false;
  auto layer = std::make_shared<
    QgsRasterLayer>( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions );
  layer->setDataProvider( "gdal" );
  return layer;
}

HeadlessRender::QgsMapLayerPtr HeadlessRender::createTemporaryLayerByType(
  const DataType type, const QgsVectorLayer::LayerOptions &layerOptions
)
{
  if ( type == DataType::Raster )
    return createTemporaryRasterLayer();
  else
    return createTemporaryVectorLayer( layerOptions );
}
