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
