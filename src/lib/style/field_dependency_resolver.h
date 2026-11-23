#ifndef QGIS_HEADLESS_FIELD_DEPENDENCY_RESOLVER_H
#define QGIS_HEADLESS_FIELD_DEPENDENCY_RESOLVER_H

#include "types.h"

#include <QSet>
#include <qgsfields.h>

class QgsVectorLayer;

namespace HeadlessRender
{
  class FieldDependencyResolver
  {
    public:
      // An unresolved dependency means the caller must keep all attributes.
      static UsedAttributes resolvePhysicalFields(
        const QgsVectorLayer &layer, const std::set<std::string> &attributes
      );

    private:
      explicit FieldDependencyResolver( const QgsVectorLayer &layer );
      bool visitField( const QString &name );
      bool visitExpressionField( int index );

      const QgsVectorLayer &mLayer;
      const QgsFields mFields;
      QSet<int> mVisiting;
      QSet<int> mResolved;
      std::set<std::string> mPhysicalFields;
  };
} //namespace HeadlessRender

#endif
