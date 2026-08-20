#ifndef QGIS_HEADLESS_MEMORY_PROVIDER_UTILS_H
#define QGIS_HEADLESS_MEMORY_PROVIDER_UTILS_H

#include <memory>

#include <qgscoordinatereferencesystem.h>
#include <qgsfeature.h>
#include <qgswkbtypes.h>

#include "types.h"

class QgsFields;

namespace HeadlessRender::MemoryProviderUtils
{
  void registerMemoryProvider();

  QgsVectorLayerPtr createMemoryLayer(
    const QString &name, const QgsFields &fields,
    Qgis::WkbType geometryType = Qgis::WkbType::NoGeometry,
    const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem(),
    bool loadDefaultStyle = true
  );
} //namespace HeadlessRender::MemoryProviderUtils

#endif
