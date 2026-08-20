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
