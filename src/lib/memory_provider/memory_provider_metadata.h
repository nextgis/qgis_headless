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

#ifndef QGIS_HEADLESS_MEMORY_PROVIDER_METADATA_H
#define QGIS_HEADLESS_MEMORY_PROVIDER_METADATA_H


#include <qgsprovidermetadata.h>


namespace HeadlessRender
{
  class MemoryProviderMetadata final : public QgsProviderMetadata
  {
      Q_OBJECT

    public:
      MemoryProviderMetadata();
      QIcon icon() const override;
      QgsDataProvider *createProvider(
        const QString &uri, const QgsDataProvider::ProviderOptions &options,
        Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags()
      ) override;
      QList< Qgis::LayerType > supportedLayerTypes() const override;
  };
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_MEMORY_PROVIDER_METADATA_H
