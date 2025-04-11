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

#ifndef QGIS_HEADLESS_UTILS_H
#define QGIS_HEADLESS_UTILS_H

#include <qgswkbtypes.h>
#include <qgsvectorlayer.h>
#include <QVariant>
#include "types.h"

namespace HeadlessRender
{
#if _QGIS_VERSION_INT < 33000
  QgsWkbTypes::Type layerGeometryTypeToQgsWkbType( HeadlessRender::LayerGeometryType geometryType );
#else
  Qgis::WkbType layerGeometryTypeToQgsWkbType( HeadlessRender::LayerGeometryType geometryType );
#endif
  QVariant::Type layerAttributeTypetoQVariantType( HeadlessRender::LayerAttributeType attributeType );

  QgsMapLayerPtr createTemporaryVectorLayer( const QgsVectorLayer::LayerOptions &layerOptions );
  QgsMapLayerPtr createTemporaryRasterLayer();
  QgsMapLayerPtr createTemporaryLayerByType(
    HeadlessRender::DataType type, const QgsVectorLayer::LayerOptions &layerOptions
  );
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_UTILS_H
