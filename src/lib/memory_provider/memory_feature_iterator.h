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

#ifndef QGIS_HEADLESS_MEMORY_FEATURE_ITERATOR_H
#define QGIS_HEADLESS_MEMORY_FEATURE_ITERATOR_H

#include <qgscoordinatetransform.h>
#include <qgsexpressioncontext.h>
#include <qgsfeatureiterator.h>
#include <qgsfields.h>
#include <qgsgeometry.h>

class QgsSpatialIndex;

namespace HeadlessRender
{
  class MemoryProvider;

  using QgsFeatureMap = QMap<QgsFeatureId, QgsFeature>;

  class MemoryFeatureSource final : public QgsAbstractFeatureSource
  {
    public:
      explicit MemoryFeatureSource( const MemoryProvider *p );

      QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

      QgsExpressionContext *expressionContext();

    private:
      QgsFields mFields;
      QgsFeatureMap mFeatures;
      std::unique_ptr< QgsSpatialIndex > mSpatialIndex;
      QString mSubsetString;
      std::unique_ptr< QgsExpressionContext > mExpressionContext;
      QgsCoordinateReferenceSystem mCrs;

      friend class MemoryFeatureIterator;
  };


  class MemoryFeatureIterator final
    : public QgsAbstractFeatureIteratorFromSource<MemoryFeatureSource>
  {
    public:
      MemoryFeatureIterator(
        MemoryFeatureSource *source, bool ownSource, const QgsFeatureRequest &request
      );

      ~MemoryFeatureIterator() override;

      bool rewind() override;
      bool close() override;

    protected:
      bool fetchFeature( QgsFeature &feature ) override;

    private:
      bool nextFeatureUsingList( QgsFeature &feature );
      bool nextFeatureTraverseAll( QgsFeature &feature );

      QgsGeometry mSelectRectGeom;
      std::unique_ptr< QgsGeometryEngine > mSelectRectEngine;
      QgsGeometry mDistanceWithinGeom;
      std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;
      QgsRectangle mFilterRect;
      QgsFeatureMap::const_iterator mSelectIterator;
      bool mUsingFeatureIdList = false;
      QList<QgsFeatureId> mFeatureIdList;
      QList<QgsFeatureId>::const_iterator mFeatureIdListIterator;
      std::unique_ptr< QgsExpression > mSubsetExpression;
      QgsCoordinateTransform mTransform;
  };
} //namespace HeadlessRender


#endif // QGIS_HEADLESS_MEMORY_FEATURE_ITERATOR_H
