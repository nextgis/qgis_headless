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
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsmemoryproviderutils.h>
#include <QByteArray>

void disableVectorSimplify( QgsVectorLayer *qgsVectorLayer )
{
    QgsVectorSimplifyMethod simplifyMethod = qgsVectorLayer->simplifyMethod();
    simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
    qgsVectorLayer->setSimplifyMethod( simplifyMethod );
}

HeadlessRender::Layer::Layer( const HeadlessRender::QgsMapLayerPtr &qgsMapLayer )
    : mLayer( qgsMapLayer )
{

}

HeadlessRender::Layer HeadlessRender::Layer::fromOgr( const std::string &uri )
{
    QgsVectorLayer *qgsVectorLayer = new QgsVectorLayer( QString::fromStdString( uri ), "", QStringLiteral( "ogr" ) );
    if ( !qgsVectorLayer->isValid() )
        throw HeadlessRender::InvalidLayerSource( "Layer source is invalid" );

    disableVectorSimplify( qgsVectorLayer );
    return Layer( QgsMapLayerPtr( qgsVectorLayer ) );
}

HeadlessRender::Layer HeadlessRender::Layer::fromGdal( const std::string &uri )
{
    QgsRasterLayer *qgsRasterLayer = new QgsRasterLayer( QString::fromStdString( uri ), "" );
    if ( !qgsRasterLayer->isValid() )
        throw HeadlessRender::InvalidLayerSource( "Layer source is invalid" );

    return Layer( QgsMapLayerPtr( qgsRasterLayer ) );
}

HeadlessRender::Layer HeadlessRender::Layer::fromData( HeadlessRender::Layer::GeometryType geometryType,
                                                       const CRS &crs,
                                                       const QVector<QPair<QString, HeadlessRender::Layer::AttributeType>> &attributeTypes,
                                                       const QVector<HeadlessRender::Layer::FeatureData> &featureDataList )
{
    QgsFields fields;
    for ( const QPair<QString, HeadlessRender::Layer::AttributeType> &attrType : attributeTypes )
        fields.append( QgsField( attrType.first, layerAttributeTypetoQVariantType( attrType.second ) ) );

    QgsVectorLayer *qgsLayer = QgsMemoryProviderUtils::createMemoryLayer( "layername", fields, layerGeometryTypeToQgsWkbType( geometryType ), *crs.qgsCoordinateReferenceSystem() );
    disableVectorSimplify( qgsLayer );

    for ( const auto &data : featureDataList )
    {
         QgsFeature feature( fields, data.id );

         QgsGeometry geom;

         size_t wkbBufSize = data.wkb.length();
         unsigned char *wkbBuf = new unsigned char[ wkbBufSize ]; // unmanaged memory is deleted in a geom.fromWkb
         memcpy( wkbBuf, data.wkb.c_str(), wkbBufSize );

         geom.fromWkb( wkbBuf, wkbBufSize );

         feature.setAttributes( QgsAttributes( data.attributes ) );
         feature.setGeometry( geom );

         qgsLayer->dataProvider()->addFeature( feature, QgsFeatureSink::FastInsert );
    }

    return Layer( QgsMapLayerPtr( qgsLayer ) );
}

HeadlessRender::QgsMapLayerPtr HeadlessRender::Layer::qgsMapLayer() const
{
    return mLayer;
}

HeadlessRender::DataType HeadlessRender::Layer::type() const
{
    if ( mLayer && mLayer->isValid() )
        mType = mLayer->type() == QgsMapLayerType::VectorLayer ? HeadlessRender::DataType::Vector : HeadlessRender::DataType::Raster;
    return mType;
}
