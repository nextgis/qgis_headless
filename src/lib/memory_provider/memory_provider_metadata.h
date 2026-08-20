/***************************************************************************
    Adapted from QGIS src/core/providers/memory/qgsmemoryprovider.h
    copyright            : (C) 2008 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
