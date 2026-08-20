/***************************************************************************
    Adapted from QGIS src/core/providers/memory/qgsmemoryprovider.cpp
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

#include "memory_provider_metadata.h"

#include <QIcon>

#include <qgsapplication.h>

#include "memory_provider.h"

using namespace HeadlessRender;

MemoryProviderMetadata::MemoryProviderMetadata()
  : QgsProviderMetadata( MemoryProvider::providerKey(), MemoryProvider::providerDescription(), MemoryProvider::providerLib() )
{}

QIcon MemoryProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconMemory.svg" ) );
}

QgsDataProvider *MemoryProviderMetadata::createProvider(
  const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags
)
{
  return new MemoryProvider( uri, options, flags );
}

QList<Qgis::LayerType> MemoryProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
}
