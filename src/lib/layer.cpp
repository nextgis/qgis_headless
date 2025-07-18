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

#include "layer.h"
#include "crs.h"
#include "utils.h"
#include "exceptions.h"
#include "style.h"
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsmemoryproviderutils.h>
#include <qgssinglesymbolrenderer.h>
#include <qgssymbol.h>
#include <QByteArray>

void disableVectorSimplify( const std::shared_ptr<QgsVectorLayer> &qgsVectorLayer )
{
  QgsVectorSimplifyMethod simplifyMethod = qgsVectorLayer->simplifyMethod();
#if _QGIS_VERSION_INT < 33800
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
#else
  simplifyMethod.setSimplifyHints(
    Qgis::VectorRenderingSimplificationFlags( Qgis::VectorRenderingSimplificationFlag::NoSimplification )
  );
#endif
  qgsVectorLayer->setSimplifyMethod( simplifyMethod );
}

HeadlessRender::Layer::Layer( const HeadlessRender::QgsMapLayerPtr &qgsMapLayer )
  : mLayer( qgsMapLayer )
{}

HeadlessRender::Layer HeadlessRender::Layer::fromOgr( const std::string &uri )
{
  QgsVectorLayer::LayerOptions layerOptions;
  layerOptions.loadDefaultStyle = false;

  auto &&qgsVectorLayer = std::make_shared<
    QgsVectorLayer>( QString::fromStdString( uri ), "", QStringLiteral( "ogr" ), layerOptions );
  if ( !qgsVectorLayer->isValid() )
    throw HeadlessRender::InvalidLayerSource( "Layer source is invalid" );

  disableVectorSimplify( qgsVectorLayer );
  return Layer( qgsVectorLayer );
}

HeadlessRender::Layer HeadlessRender::Layer::fromGdal( const std::string &uri )
{
  auto &&qgsRasterLayer = std::make_shared<QgsRasterLayer>( QString::fromStdString( uri ) );
  if ( !qgsRasterLayer->isValid() )
    throw HeadlessRender::InvalidLayerSource(
      "Layer source is invalid, error message: "
      + qgsRasterLayer->error().message( QgsErrorMessage::Text )
    );

  return Layer( qgsRasterLayer );
}

HeadlessRender::Layer HeadlessRender::Layer::fromData(
  HeadlessRender::LayerGeometryType geometryType, const CRS &crs,
  const QVector<QPair<QString, HeadlessRender::LayerAttributeType>> &attributeTypes,
  const QVector<HeadlessRender::Layer::FeatureData> &featureDataList
)
{
  QgsFields fields;
  for ( const QPair<QString, HeadlessRender::LayerAttributeType> &attrType : attributeTypes )
    fields.append( QgsField( attrType.first, layerAttributeTypetoQVariantType( attrType.second ) ) );

  std::shared_ptr<QgsVectorLayer> qgsLayer(
    QgsMemoryProviderUtils::
      createMemoryLayer( "layername", fields, layerGeometryTypeToQgsWkbType( geometryType ), *crs.qgsCoordinateReferenceSystem() )
  );
  disableVectorSimplify( qgsLayer );

  for ( const auto &data : featureDataList )
  {
    QgsFeature feature( fields, data.id );

    QgsGeometry geom;
    geom.fromWkb( QByteArray::fromStdString( data.wkb ) );

    feature.setAttributes( QgsAttributes( data.attributes ) );
    feature.setGeometry( geom );

    qgsLayer->dataProvider()->addFeature( feature, QgsFeatureSink::FastInsert );
  }

  return Layer( qgsLayer );
}

HeadlessRender::QgsMapLayerPtr HeadlessRender::Layer::qgsMapLayer() const
{
  return mLayer;
}

HeadlessRender::DataType HeadlessRender::Layer::type() const
{
  if ( mLayer && mLayer->isValid() )
#if _QGIS_VERSION_INT < 33000
    mType = mLayer->type() == QgsMapLayerType::VectorLayer ? HeadlessRender::DataType::Vector
                                                           : HeadlessRender::DataType::Raster;
#else
    mType = mLayer->type() == Qgis::LayerType::Vector ? HeadlessRender::DataType::Vector
                                                      : HeadlessRender::DataType::Raster;
#endif
  return mType;
}

void HeadlessRender::Layer::setRendererSymbolColor( const QColor &color )
{
  if ( type() != HeadlessRender::DataType::Vector )
    return;

  auto &&layer = std::dynamic_pointer_cast< QgsVectorLayer >( mLayer );
  if ( !layer )
    return;

  QgsSingleSymbolRenderer *singleRenderer = dynamic_cast< QgsSingleSymbolRenderer * >(
    layer->renderer()
  );
  std::unique_ptr<QgsSymbol> newSymbol;

  if ( singleRenderer && singleRenderer->symbol() )
    newSymbol.reset( singleRenderer->symbol()->clone() );

  const QgsSingleSymbolRenderer *embeddedRenderer = nullptr;
  if ( !newSymbol && layer->renderer()->embeddedRenderer() )
  {
    embeddedRenderer = dynamic_cast< const QgsSingleSymbolRenderer * >(
      layer->renderer()->embeddedRenderer()
    );
    if ( embeddedRenderer && embeddedRenderer->symbol() )
      newSymbol.reset( embeddedRenderer->symbol()->clone() );
  }

  if ( newSymbol )
  {
    newSymbol->setColor( color );
    if ( singleRenderer )
    {
      singleRenderer->setSymbol( newSymbol.release() );
    }
    else if ( embeddedRenderer )
    {
      std::unique_ptr<QgsSingleSymbolRenderer> newRenderer( embeddedRenderer->clone() );
      newRenderer->setSymbol( newSymbol.release() );
      layer->renderer()->setEmbeddedRenderer( newRenderer.release() );
    }
  }
}

bool HeadlessRender::Layer::addStyle( HeadlessRender::Style &style, QString &error )
{
  if ( type() != style.type() )
    throw StyleTypeMismatch( "Layer type and style type do not match" );

  return style.importToLayer( mLayer, error );
}
