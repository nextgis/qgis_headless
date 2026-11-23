/***************************************************************************
    Adapted from QGIS src/core/providers/memory/qgsmemoryfeatureiterator.h
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGIS_HEADLESS_MEMORY_FEATURE_ITERATOR_H
#define QGIS_HEADLESS_MEMORY_FEATURE_ITERATOR_H

#include "memory_feature_map_p.h"

#include <qgscoordinatetransform.h>
#include <qgsexpressioncontext.h>
#include <qgsfeatureiterator.h>
#include <qgsfields.h>
#include <qgsgeometry.h>

class QgsSpatialIndex;

namespace HeadlessRender
{
  class MemoryProvider;

  class MemoryFeatureSource final : public QgsAbstractFeatureSource
  {
    public:
      explicit MemoryFeatureSource( const MemoryProvider *p );

      QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

      QgsExpressionContext *expressionContext();

    private:
      QgsFields mFields;
      detail::MemoryFeatureMap mFeatures;
      QList<QgsFeatureId> mFeatureOrder;
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
      QList<QgsFeatureId>::const_iterator mSelectIterator;
      bool mUsingSpatialIndex = false;
      bool mUsingFeatureIdList = false;
      QList<QgsFeatureId> mFeatureIdList;
      QList<QgsFeatureId>::const_iterator mFeatureIdListIterator;
      std::unique_ptr< QgsExpression > mSubsetExpression;
      QgsCoordinateTransform mTransform;
  };
} //namespace HeadlessRender


#endif // QGIS_HEADLESS_MEMORY_FEATURE_ITERATOR_H
