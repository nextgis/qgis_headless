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

#ifndef QGIS_HEADLESS_MEMORY_PROVIDER_H
#define QGIS_HEADLESS_MEMORY_PROVIDER_H


#include <qgscoordinatereferencesystem.h>
#include <qgsfields.h>
#include <qgsvectordataprovider.h>

class QgsSpatialIndex;

namespace HeadlessRender
{
  class MemoryFeatureIterator;

  class MemoryProvider final : public QgsVectorDataProvider
  {
      Q_OBJECT

    public:
      using Capabilities = Qgis::VectorProviderCapabilities;

    public:
      explicit MemoryProvider(
        const QString &uri, const QgsVectorDataProvider::ProviderOptions &options,
        Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags()
      );

      ~MemoryProvider() override;

      //! Returns the memory provider key
      static QString providerKey();
      //! Returns the memory provider description
      static QString providerDescription();

      static QString providerLib();

      /* Implementation of functions from QgsVectorDataProvider */

      QgsAbstractFeatureSource *featureSource() const override;

      QString dataSourceUri( bool expandAuthConfig = true ) const override;
      QString storageType() const override;
      QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
      Qgis::WkbType wkbType() const override;
      long long featureCount() const override;
      QgsFields fields() const override;
      bool addFeatures(
        QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags()
      ) override;
      bool deleteFeatures( const QgsFeatureIds &id ) override;
      bool addAttributes( const QList<QgsField> &attributes ) override;
      bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
      bool deleteAttributes( const QgsAttributeIds &attributes ) override;
      bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
      bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
      bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
      bool supportsSubsetString() const override;
      QString subsetString() const override;
      QString subsetStringDialect() const override;
      QString subsetStringHelpUrl() const override;
      Qgis::SpatialIndexPresence hasSpatialIndex() const override;
      bool createSpatialIndex() override;
      Capabilities capabilities() const override;
      bool truncate() override;

      /* Implementation of functions from QgsDataProvider */

      QString name() const override;
      QString description() const override;
      QgsRectangle extent() const override;
      void updateExtents() override;
      bool isValid() const override;
      QgsCoordinateReferenceSystem crs() const override;
      void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

    private:
      using FeatureMap = QMap<QgsFeatureId, QgsFeature>;

    private:
      // Coordinate reference system
      QgsCoordinateReferenceSystem mCrs;

      // fields
      QgsFields mFields;
      Qgis::WkbType mWkbType;
      mutable QgsRectangle mExtent;

      // features
      FeatureMap mFeatures;
      QgsFeatureId mNextFeatureId;

      // indexing
      std::unique_ptr<QgsSpatialIndex> mSpatialIndex;

      QString mSubsetString;

      friend class MemoryFeatureSource;
  };
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_MEMORY_PROVIDER_H
