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

QgsWkbTypes::Type toQgsWkbType( HeadlessRender::Layer::GeometryType geometryType )
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
    }
}

QVariant::Type HeadlessRender::Layer::toQVariantType( HeadlessRender::Layer::AttributeType attributeType )
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

HeadlessRender::Layer::Layer( const HeadlessRender::QgsMapLayerPtr &qgsMapLayer )
    : mLayer( qgsMapLayer )
{

}

HeadlessRender::Layer HeadlessRender::Layer::fromOgr( const std::string &uri )
{
    QgsVectorLayer *qgsVectorLayer = new QgsVectorLayer( QString::fromStdString( uri ), "", QStringLiteral( "ogr" ) );
    disableVectorSimplify( qgsVectorLayer );
    return Layer( QgsMapLayerPtr( qgsVectorLayer ) );
}

HeadlessRender::Layer HeadlessRender::Layer::fromGdal( const std::string &uri )
{
    return Layer( QgsMapLayerPtr( new QgsRasterLayer( QString::fromStdString( uri ), "" ) ) );
}

HeadlessRender::Layer HeadlessRender::Layer::fromData( HeadlessRender::Layer::GeometryType geometryType,
                                                       const CRS &crs,
                                                       const QVector<QPair<QString, HeadlessRender::Layer::AttributeType>> &attributeTypes,
                                                       const QVector<HeadlessRender::Layer::FeatureData> &featureDataList )
{
    QgsFields fields;
    for ( const QPair<QString, HeadlessRender::Layer::AttributeType> &attrType : attributeTypes )
        fields.append( QgsField( attrType.first, toQVariantType( attrType.second ) ) );

    QgsVectorLayer *qgsLayer = QgsMemoryProviderUtils::createMemoryLayer( "layername", fields, toQgsWkbType( geometryType ), *crs.qgsCoordinateReferenceSystem() );
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
