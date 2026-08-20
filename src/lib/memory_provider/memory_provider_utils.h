/***************************************************************************
    Adapted from QGIS src/core/providers/memory/qgsmemoryproviderutils.h
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGIS_HEADLESS_MEMORY_PROVIDER_UTILS_H
#define QGIS_HEADLESS_MEMORY_PROVIDER_UTILS_H

#include <memory>

#include <qgscoordinatereferencesystem.h>
#include <qgsfeature.h>
#include <qgswkbtypes.h>

class QgsFields;
class QgsVectorLayer;

namespace HeadlessRender::MemoryProviderUtils
{
  void registerMemoryProvider();

  std::shared_ptr<QgsVectorLayer> createMemoryLayer(
    const QString &name, const QgsFields &fields,
    Qgis::WkbType geometryType = Qgis::WkbType::NoGeometry,
    const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem(),
    bool loadDefaultStyle = true
  );
} //namespace HeadlessRender::MemoryProviderUtils

#endif
