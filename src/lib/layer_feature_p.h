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
    auto *provider = layer.dataProvider();
    // The feature is local, so the provider can update it without taking a copy.
    if ( !provider->addFeature( feature ) )
      throw std::invalid_argument(
        QStringLiteral( "Cannot insert feature %1 into the memory provider: %2" )
          .arg( id )
          .arg( provider->errors().join( QStringLiteral( "; " ) ) )
          .toStdString()
      );
  }
} //namespace HeadlessRender::detail

#endif // QGIS_HEADLESS_LAYER_FEATURE_P_H
