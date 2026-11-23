#include "schema_raster_provider.h"
#include "layer_schema.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <qgsprovidermetadata.h>
#include <qgsproviderregistry.h>
#include <qgsrasterblock.h>
#include <qgsrasterdataprovider.h>
#include <qgsrasterlayer.h>
#include <stdexcept>

namespace
{
  const QString ProviderKey = QStringLiteral( "qgis_headless_schema" );

  // Import-time metadata only. Statistics and block reads never synthesize data
  // or open files/network connections. Rendered requests use their real provider.
  class SchemaRasterProvider final : public QgsRasterDataProvider
  {
    public:
      SchemaRasterProvider(
        const QString &uri, const ProviderOptions &options, Qgis::DataProviderReadFlags flags
      )
        : QgsRasterDataProvider( uri, options, flags )
      {
        const QJsonObject data = QJsonDocument::fromJson( QByteArray::fromBase64( uri.toUtf8() ) )
                                   .object();
        mCrs.createFromWkt( data.value( "crs" ).toString() );
        mWidth = data.value( "width" ).toInt();
        mHeight = data.value( "height" ).toInt();
        for ( const auto &band : data.value( "bands" ).toArray() )
          mTypes.append( static_cast<Qgis::DataType>( band.toInt() ) );
      }

      SchemaRasterProvider *clone() const override
      {
        return new SchemaRasterProvider( dataSourceUri(), ProviderOptions(), Qgis::DataProviderReadFlags() );
      }
      QString name() const override
      {
        return ProviderKey;
      }
      QString description() const override
      {
        return QStringLiteral( "Headless style schema" );
      }
      bool isValid() const override
      {
        return mWidth > 0 && mHeight > 0 && !mTypes.isEmpty();
      }
      QgsCoordinateReferenceSystem crs() const override
      {
        return mCrs;
      }
      QgsRectangle extent() const override
      {
        return QgsRectangle( 0, 0, mWidth, mHeight );
      }
      int xSize() const override
      {
        return mWidth;
      }
      int ySize() const override
      {
        return mHeight;
      }
      int bandCount() const override
      {
        return mTypes.size();
      }
      Qgis::DataType dataType( int band ) const override
      {
        return band > 0 && band <= mTypes.size() ? mTypes[band - 1]
                                                 : Qgis::DataType::UnknownDataType;
      }
      Qgis::DataType sourceDataType( int band ) const override
      {
        return dataType( band );
      }
      Qgis::RasterColorInterpretation colorInterpretation( int band ) const override
      {
        if ( mTypes.size() >= 3 )
        {
          if ( band == 1 )
            return Qgis::RasterColorInterpretation::RedBand;
          if ( band == 2 )
            return Qgis::RasterColorInterpretation::GreenBand;
          if ( band == 3 )
            return Qgis::RasterColorInterpretation::BlueBand;
        }
        return Qgis::RasterColorInterpretation::GrayIndex;
      }
      QString lastErrorTitle() override
      {
        return {};
      }
      QString lastError() override
      {
        return {};
      }
      QgsRasterBlock *block( int, const QgsRectangle &, int, int, QgsRasterBlockFeedback * ) override
      {
        return new QgsRasterBlock;
      }
      QgsRasterBandStats bandStatistics(
        int, Qgis::RasterBandStatistics, const QgsRectangle &, int, QgsRasterBlockFeedback *
      ) override
      {
        return {};
      }
      bool enableProviderResampling( bool enabled ) override
      {
        mProviderResamplingEnabled = enabled;
        return true;
      }
      bool setZoomedInResamplingMethod( Qgis::RasterResamplingMethod method ) override
      {
        mZoomedInResamplingMethod = method;
        return true;
      }
      bool setZoomedOutResamplingMethod( Qgis::RasterResamplingMethod method ) override
      {
        mZoomedOutResamplingMethod = method;
        return true;
      }
      bool setMaxOversampling( double value ) override
      {
        mMaxOversampling = value;
        return true;
      }

    private:
      QgsCoordinateReferenceSystem mCrs;
      QVector<Qgis::DataType> mTypes;
      int mWidth = 0;
      int mHeight = 0;
  };

  class SchemaProviderMetadata final : public QgsProviderMetadata
  {
    public:
      SchemaProviderMetadata()
        : QgsProviderMetadata( ProviderKey, QStringLiteral( "Headless style schema" ) )
      {}
      QgsDataProvider *createProvider(
        const QString &uri, const QgsDataProvider::ProviderOptions &options,
        Qgis::DataProviderReadFlags flags
      ) override
      {
        return new SchemaRasterProvider( uri, options, flags );
      }
  };
} //namespace

HeadlessRender::QgsMapLayerPtr HeadlessRender::createSchemaRasterLayer( const LayerSchema &schema )
{
  auto *registry = QgsProviderRegistry::instance();
  if ( !registry->providerMetadata( ProviderKey ) )
  {
    auto metadata = std::make_unique<SchemaProviderMetadata>();
    if ( !registry->registerProvider( metadata.get() ) )
      throw std::runtime_error( "Cannot register the style schema provider" );
    metadata.release();
  }
  QJsonArray bands;
  for ( const auto type : schema.bandTypes() )
    bands.append( static_cast<int>( type ) );
  const QJsonObject
    data { { "crs", schema.crs().toWkt() }, { "width", schema.width() }, { "height", schema.height() }, { "bands", bands } };
  const QString uri = QString::fromUtf8(
    QJsonDocument( data ).toJson( QJsonDocument::Compact ).toBase64()
  );
  QgsRasterLayer::LayerOptions options;
  options.loadDefaultStyle = false;
  return std::make_shared<QgsRasterLayer>( uri, QStringLiteral( "style" ), ProviderKey, options );
}
