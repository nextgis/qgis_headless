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
#include "types.h"

class QgsMapLayer;

namespace HeadlessRender
{
  class Style;
  typedef std::shared_ptr<QgsMapLayer> QgsMapLayerPtr;

  class QGIS_HEADLESS_EXPORT Layer
  {
    public:
      struct FeatureData
      {
          qint64 id;
          std::string wkb;
          QVector<QVariant> attributes;
      };

      Layer() = default;

      static Layer fromOgr( const std::string &uri );
      static Layer fromGdal( const std::string &uri );
      static Layer fromData(
        LayerGeometryType geometryType, const CRS &crs,
        const QVector<QPair<QString, LayerAttributeType>> &attributeTypes,
        const QVector<FeatureData> &featureDataList
      );

      QgsMapLayerPtr qgsMapLayer() const;
      DataType type() const;
      void setRendererSymbolColor( const QColor &color );
      bool addStyle( Style &style, QString &error );

    private:
      explicit Layer( const QgsMapLayerPtr &qgsMapLayer );

      QgsMapLayerPtr mLayer;
      mutable DataType mType = DataType::Unknown;

      friend class Project;
  };
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_LAYER_H
