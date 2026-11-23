#ifndef QGIS_HEADLESS_LAYER_FEATURE_P_H
#define QGIS_HEADLESS_LAYER_FEATURE_P_H

#include <qgsvectorlayer.h>
#include <stdexcept>

namespace HeadlessRender::detail
{
  inline void addMemoryLayerFeature(
    QgsVectorLayer &layer, const QgsFields &fields, qint64 id, const QByteArray &wkb,
    const QgsAttributes &attributes
  )
  {
    QgsFeature feature( fields, id );
    QgsGeometry geometry;
    geometry.fromWkb( wkb );
    feature.setGeometry( geometry );
    feature.setAttributes( attributes );
    if ( !layer.dataProvider()->addFeature( feature, QgsFeatureSink::FastInsert ) )
      throw std::invalid_argument( "Cannot insert feature into the memory provider" );
  }
} //namespace HeadlessRender::detail

#endif // QGIS_HEADLESS_LAYER_FEATURE_P_H
