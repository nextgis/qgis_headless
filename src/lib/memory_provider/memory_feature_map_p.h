#ifndef QGIS_HEADLESS_MEMORY_FEATURE_MAP_P_H
#define QGIS_HEADLESS_MEMORY_FEATURE_MAP_P_H

#include <QHash>
#include <qgsfeature.h>

namespace HeadlessRender::detail
{
  struct MemoryFeatureKey
  {
      QgsFeatureId id;

      MemoryFeatureKey( QgsFeatureId id ) noexcept : id( id )
      {}

      bool operator==( const MemoryFeatureKey &other ) const noexcept
      {
        return id == other.id;
      }
  };

  inline uint qHash( MemoryFeatureKey key, uint seed = 0 ) noexcept
  {
    // Qt 5 folds 64-bit integers with XOR, which can collapse correlated ID halves.
    // Mix the whole ID with the MurmurHash3 64-bit finalizer before narrowing.
    quint64 value = static_cast<quint64>( key.id ) ^ seed;
    value ^= value >> 33;
    value *= Q_UINT64_C( 0xff51afd7ed558ccd );
    value ^= value >> 33;
    value *= Q_UINT64_C( 0xc4ceb9fe1a85ec53 );
    value ^= value >> 33;
    return static_cast<uint>( value );
  }

  using MemoryFeatureMap = QHash<MemoryFeatureKey, QgsFeature>;
} //namespace HeadlessRender::detail

#endif // QGIS_HEADLESS_MEMORY_FEATURE_MAP_P_H
