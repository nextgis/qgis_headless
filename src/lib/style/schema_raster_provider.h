#ifndef QGIS_HEADLESS_SCHEMA_RASTER_PROVIDER_H
#define QGIS_HEADLESS_SCHEMA_RASTER_PROVIDER_H

#include "types.h"

namespace HeadlessRender
{
  class LayerSchema;
  QgsMapLayerPtr createSchemaRasterLayer( const LayerSchema &schema );
} //namespace HeadlessRender

#endif
