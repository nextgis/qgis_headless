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
#include "exceptions.h"

#if _QGIS_VERSION_INT >= 33000
#include "memory_provider/memory_provider_utils.h"
#else
#include <qgsmemoryproviderutils.h>
#endif

using namespace HeadlessRender;

QgisGeometryType HeadlessRender::layerGeometryTypeToQgsWkbType( LayerGeometryType geometryType )
{
  switch ( geometryType )
  {
    case LayerGeometryType::Point:
      return QgisGeometryType::Point;
    case LayerGeometryType::LineString:
      return QgisGeometryType::LineString;
    case LayerGeometryType::Polygon:
      return QgisGeometryType::Polygon;
    case LayerGeometryType::MultiPoint:
      return QgisGeometryType::MultiPoint;
    case LayerGeometryType::MultiLineString:
      return QgisGeometryType::MultiLineString;
    case LayerGeometryType::MultiPolygon:
      return QgisGeometryType::MultiPolygon;
    case LayerGeometryType::PointZ:
      return QgisGeometryType::PointZ;
    case LayerGeometryType::LineStringZ:
      return QgisGeometryType::LineStringZ;
    case LayerGeometryType::PolygonZ:
      return QgisGeometryType::PolygonZ;
    case LayerGeometryType::MultiPointZ:
      return QgisGeometryType::MultiPointZ;
    case LayerGeometryType::MultiLineStringZ:
      return QgisGeometryType::MultiLineStringZ;
    case LayerGeometryType::MultiPolygonZ:
      return QgisGeometryType::MultiPolygonZ;
    case LayerGeometryType::Unknown:
      return QgisGeometryType::Unknown;
  }
  return QgisGeometryType::Unknown;
}

QVariant::Type HeadlessRender::layerAttributeTypetoQVariantType( LayerAttributeType attributeType )
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

QgsMapLayerPtr HeadlessRender::createTemporaryVectorLayer( const QgsVectorLayer::LayerOptions &layerOptions )
{
  return std::make_shared<
    QgsVectorLayer>( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions );
}

QgsMapLayerPtr HeadlessRender::createTemporaryRasterLayer()
{
  QgsRasterLayer::LayerOptions layerOptions;
  layerOptions.loadDefaultStyle = false;
  auto layer = std::make_shared<
    QgsRasterLayer>( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions );
  layer->setDataProvider( "gdal" );
  return layer;
}

QgsMapLayerPtr HeadlessRender::createTemporaryLayerByType(
  const DataType type, const QgsVectorLayer::LayerOptions &layerOptions
)
{
  if ( type == DataType::Raster )
    return createTemporaryRasterLayer();
  else
    return createTemporaryVectorLayer( layerOptions );
}

QgsVectorLayerPtr HeadlessRender::createMemoryLayer(
  QgsFeatureList &features, const QgsFields &fields, QgisGeometryType geometryType,
  const QgsCoordinateReferenceSystem &crs
)
{
#if _QGIS_VERSION_INT >= 33000
  auto layer = MemoryProviderUtils::createMemoryLayer( "layername", fields, geometryType, crs );
#else
  QgsVectorLayerPtr layer(
    QgsMemoryProviderUtils::createMemoryLayer( "layername", fields, geometryType, crs )
  );
#endif

  disableVectorSimplify( layer );
  if ( !layer->dataProvider()
       || !layer->dataProvider()->addFeatures( features, QgsFeatureSink::RollBackOnErrors ) )
  {
    throw MemoryLayerError( "An error occurred while cloning features to the memory layer" );
  }
  return layer;
}

QgsVectorLayerPtr HeadlessRender::cloneLayerToMemory( const QgsVectorLayerPtr &layer )
{
  auto &&featureIterator = layer->getFeatures();
  QgsFeatureList features;

  QgsFeature currentFeature;
  while ( featureIterator.nextFeature( currentFeature ) )
  {
    features.append( currentFeature );
  }

  return createMemoryLayer( features, layer->fields(), layer->wkbType(), layer->crs() );
}

void HeadlessRender::disableVectorSimplify( const std::shared_ptr<QgsVectorLayer> &qgsVectorLayer )
{
  QgsVectorSimplifyMethod simplifyMethod = qgsVectorLayer->simplifyMethod();
#if _QGIS_VERSION_INT >= 33800
  simplifyMethod.setSimplifyHints(
    Qgis::VectorRenderingSimplificationFlags( Qgis::VectorRenderingSimplificationFlag::NoSimplification )
  );
#else
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
#endif
  qgsVectorLayer->setSimplifyMethod( simplifyMethod );
}
