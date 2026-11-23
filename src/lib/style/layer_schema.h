#ifndef QGIS_HEADLESS_LAYER_SCHEMA_H
#define QGIS_HEADLESS_LAYER_SCHEMA_H

#include "crs.h"
#include "types.h"

#include <qgscoordinatereferencesystem.h>
#include <qgsfields.h>
#include <vector>

class QgsRasterLayer;

namespace HeadlessRender
{
  /** Describe source metadata without retaining features, pixels or an open data source.
   * Instances are values: Style retains its own schema snapshot.
   */
  class QGIS_HEADLESS_EXPORT LayerSchema
  {
    public:
      /** Describe the complete physical field list, before request-time projection.
       * Expression fields defined by QML are added by QGIS during style import.
       * Throws std::invalid_argument for unknown geometry or duplicate/empty field names.
       */
      static LayerSchema vector(
        const CRS &crs, LayerGeometryType geometryType,
        const std::vector<std::pair<std::string, LayerAttributeType>> &fields
      );

      /** Describe raster bands in their original order using QGIS data-type names.
       * Width/height must be positive. This does not allocate a raster of that size.
       * Throws std::invalid_argument for unsupported band types or invalid dimensions.
       */
      static LayerSchema raster(
        const CRS &crs, const std::vector<std::string> &bandTypes, int width, int height
      );

      DataType type() const
      {
        return mType;
      }
      const QgsFields &fields() const
      {
        return mFields;
      }
      Qgis::WkbType wkbType() const
      {
        return mGeometry;
      }
      const QgsCoordinateReferenceSystem &crs() const
      {
        return mCrs;
      }
      const QVector<Qgis::DataType> &bandTypes() const
      {
        return mBandTypes;
      }
      int width() const
      {
        return mWidth;
      }
      int height() const
      {
        return mHeight;
      }

      /** Create an empty vector layer or a metadata-only raster layer for QGIS import. */
      QgsMapLayerPtr createLayer() const;

      /** Capture raster metadata without reading pixels or computing statistics. */
      static LayerSchema fromRaster( const QgsRasterLayer &layer );
      /** Check the band layout used to select a cached raster renderer. */
      bool matchesRaster( const QgsRasterLayer &layer ) const;

    private:
      LayerSchema() = default;
      DataType mType = DataType::Unknown;
      QgsCoordinateReferenceSystem mCrs;
      Qgis::WkbType mGeometry = Qgis::WkbType::Unknown;
      QgsFields mFields;
      QVector<Qgis::DataType> mBandTypes;
      int mWidth = 0;
      int mHeight = 0;
  };
} // namespace HeadlessRender

#endif
