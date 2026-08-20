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
