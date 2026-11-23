#include "layer_schema.h"
#include "exceptions.h"
#include "schema_raster_provider.h"
#include "utils.h"

#include <qgsmemoryproviderutils.h>
#include <qgsrasterdataprovider.h>
#include <qgsrasterlayer.h>
#include <qgsvectorlayer.h>
#include <stdexcept>

using namespace HeadlessRender;

namespace
{
  QgsCoordinateReferenceSystem schemaCrs( const CRS &crs )
  {
    const auto definition = crs.qgsCoordinateReferenceSystem();
    if ( !definition || !definition->isValid() )
      throw InvalidCRSError( QStringLiteral( "Layer schema requires a valid CRS" ) );
    return *definition;
  }
} //namespace

LayerSchema LayerSchema::vector(
  const CRS &crs, LayerGeometryType geometryType,
  const std::vector<std::pair<std::string, LayerAttributeType>> &fields
)
{
  LayerSchema result;
  result.mType = DataType::Vector;
  result.mCrs = schemaCrs( crs );
  result.mGeometry = layerGeometryTypeToQgsWkbType( geometryType );
  if ( result.mGeometry == Qgis::WkbType::Unknown )
    throw std::invalid_argument( "Vector schema requires a known geometry type" );
  for ( const auto &field : fields )
  {
    const QString name = QString::fromStdString( field.first );
    if ( name.isEmpty() || result.mFields.indexOf( name ) >= 0 )
      throw std::invalid_argument( "Schema field names must be nonempty and unique" );
    result.mFields.append( QgsField( name, layerAttributeTypetoQVariantType( field.second ) ) );
  }
  return result;
}

LayerSchema LayerSchema::raster(
  const CRS &crs, const std::vector<std::string> &bandTypes, int width, int height
)
{
  if ( width <= 0 || height <= 0 || bandTypes.empty() )
    throw std::invalid_argument( "Raster schema requires positive dimensions and at least one band" );
  LayerSchema result;
  result.mType = DataType::Raster;
  result.mCrs = schemaCrs( crs );
  result.mWidth = width;
  result.mHeight = height;
  for ( const auto &name : bandTypes )
  {
    const auto type
      = qgsEnumKeyToValue( QString::fromStdString( name ), Qgis::DataType::UnknownDataType );
    if ( type == Qgis::DataType::UnknownDataType )
      throw std::invalid_argument( "Unknown raster band data type: " + name );
    result.mBandTypes.append( type );
  }
  return result;
}

QgsMapLayerPtr LayerSchema::createLayer() const
{
  if ( mType == DataType::Vector )
    return QgsMapLayerPtr(
      QgsMemoryProviderUtils::createMemoryLayer( QStringLiteral( "style" ), mFields, mGeometry, mCrs, false )
    );
  return createSchemaRasterLayer( *this );
}

LayerSchema LayerSchema::fromRaster( const QgsRasterLayer &layer )
{
  LayerSchema result;
  result.mType = DataType::Raster;
  result.mCrs = layer.crs();
  result.mWidth = layer.width();
  result.mHeight = layer.height();
  if ( const auto *provider = layer.dataProvider() )
  {
    for ( int band = 1; band <= provider->bandCount(); ++band )
      result.mBandTypes.append( provider->dataType( band ) );
  }
  return result;
}

bool LayerSchema::matchesRaster( const QgsRasterLayer &layer ) const
{
  const auto *provider = layer.dataProvider();
  if ( mType != DataType::Raster || !provider || provider->bandCount() != mBandTypes.size() )
    return false;
  for ( int band = 1; band <= provider->bandCount(); ++band )
  {
    if ( provider->dataType( band ) != mBandTypes[band - 1] )
      return false;
  }
  return true;
}
