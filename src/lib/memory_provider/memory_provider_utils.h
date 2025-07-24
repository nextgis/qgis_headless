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
