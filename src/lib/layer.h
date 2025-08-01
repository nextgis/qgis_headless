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

  /**
   * Represents a map layer (supports both vector and raster layer types).
   */
  class QGIS_HEADLESS_EXPORT Layer
  {
    public:
      /**
       * Represents data of vector layer's object.
       */
      struct FeatureData
      {
          qint64 id;
          std::string wkb;
          QVector<QVariant> attributes;
      };

      /**
       * Creates a vector layer from a data source.
       * \param uri points to a data source (e.g., file, database, or service) with vector layer.
       * \returns vector layer, loaded from the specified data source.
       */
      static Layer fromOgr( const std::string &uri );

      /**
       * Creates a raster layer from a data source.
       * \param uri points to a data source (e.g., file, database, or service) with raster layer.
       * \returns raster layer, loaded from the specified data source.
       */
      static Layer fromGdal( const std::string &uri );

      /**
       * Creates a vector layer of given geometry type, CRS with attributes of given types from QVector of FeatureList
       * \param geometryType type of geometry for vector layer.
       * \param crs CRS of layer.
       * \param attributeTypes names and types of attributive data of layer.
       * \param featureDataList spatial and attributive data of layer's objects.
       * \returns new vector non-file related layer.
       */
      static Layer fromData(
        LayerGeometryType geometryType, const CRS &crs,
        const QVector<QPair<QString, LayerAttributeType>> &attributeTypes,
        const QVector<FeatureData> &featureDataList
      );

      /**
       * Returns a shared_ptr to the underlying QgsMapLayer object.
       */
      QgsMapLayerPtr qgsMapLayer() const;

      /**
       * Returns type of layer: raster or vector.
       */
      DataType type() const;

      /**
       * Applies style to layer.
       * \param style the style that will be applied.
       */
      void addStyle( const Style &style );

    private:
      explicit Layer( const QgsMapLayerPtr &qgsMapLayer );

      QgsMapLayerPtr mLayer;
      mutable DataType mType = DataType::Unknown;

      friend class Project;
  };
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_LAYER_H
