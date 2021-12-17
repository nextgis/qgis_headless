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

#ifndef QGIS_HEADLESS_LAYER_H
#define QGIS_HEADLESS_LAYER_H

#include <string>
#include <QVariant>
#include <QString>
#include <QVector>
#include "crs.h"

class QgsMapLayer;

namespace HeadlessRender
{
    typedef std::shared_ptr<QgsMapLayer> QgsMapLayerPtr;

    class QGIS_HEADLESS_EXPORT Layer
    {
    public:
        enum class GeometryType
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

        enum class AttributeType
        {
            Integer,
            Real,
            String,
            Date,
            Time,
            DateTime,
            Integer64
        };

        struct FeatureData
        {
            qint64 id;
            std::string wkb;
            QVector<QVariant> attributes;
        };

        Layer() = default;
        explicit Layer( const QgsMapLayerPtr &qgsMapLayer );

        static Layer fromOgr( const std::string &uri );
        static Layer fromGdal( const std::string &uri );
        static Layer fromData( GeometryType geometryType, const CRS &crs, const QVector<QPair<QString, AttributeType>> &attributeTypes, const QVector<FeatureData> &featureDataList );

        static QVariant::Type toQVariantType( AttributeType attributeType );

        QgsMapLayerPtr qgsMapLayer() const;
    private:
        QgsMapLayerPtr mLayer;
    };
}

#endif // QGIS_HEADLESS_LAYER_H
