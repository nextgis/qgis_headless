#include "memory_provider_utils.h"

#include <QUrl>

#include <qgsfields.h>
#include <qgsproviderregistry.h>
#include <qgsvectorlayer.h>

#include "exceptions.h"
#include "memory_provider.h"
#include "memory_provider_metadata.h"

using namespace HeadlessRender;

void MemoryProviderUtils::registerMemoryProvider()
{
  auto provider = std::make_unique<MemoryProviderMetadata>();
  if ( !QgsProviderRegistry::instance()->registerProvider( provider.get() ) )
  {
    // If an error occurred, provider will not be owned by QgsProviderRegistry
    throw QgisHeadlessError( "QGIS Headless memory provider registry error" );
  }
  provider.release();
}

static QString memoryLayerFieldType( QMetaType::Type type, const QString &typeString )
{
  switch ( type )
  {
    case QMetaType::Type::Int:
      return QStringLiteral( "integer" );
    case QMetaType::Type::LongLong:
      return QStringLiteral( "long" );
    case QMetaType::Type::Double:
      return QStringLiteral( "double" );
    case QMetaType::Type::QString:
      return QStringLiteral( "string" );
    case QMetaType::Type::QDate:
      return QStringLiteral( "date" );
    case QMetaType::Type::QTime:
      return QStringLiteral( "time" );
    case QMetaType::Type::QDateTime:
      return QStringLiteral( "datetime" );
    case QMetaType::Type::QByteArray:
      return QStringLiteral( "binary" );
    case QMetaType::Type::Bool:
      return QStringLiteral( "boolean" );
    case QMetaType::Type::QVariantMap:
      return QStringLiteral( "map" );
    case QMetaType::Type::User:
      if ( typeString.compare( QLatin1String( "geometry" ), Qt::CaseInsensitive ) == 0 )
      {
        return QStringLiteral( "geometry" );
      }
      break;
    default:
      break;
  }
  return QStringLiteral( "string" );
}

QgsVectorLayerPtr MemoryProviderUtils::createMemoryLayer(
  const QString &name, const QgsFields &fields, Qgis::WkbType geometryType,
  const QgsCoordinateReferenceSystem &crs, bool loadDefaultStyle
)
{
  QString geomType = QgsWkbTypes::displayString( geometryType );
  if ( geomType.isNull() )
  {
    geomType = QStringLiteral( "none" );
  }

  QStringList parts;
  if ( crs.isValid() )
  {
    if ( !crs.authid().isEmpty() )
    {
      parts << QStringLiteral( "crs=%1" ).arg( crs.authid() );
    }
    else
    {
      parts << QStringLiteral( "crs=wkt:%1" ).arg( crs.toWkt( Qgis::CrsWktVariant::Preferred ) );
    }
  }
  else
  {
    parts << QStringLiteral( "crs=" );
  }
  for ( const QgsField &field : fields )
  {
    const QString lengthPrecision
      = QStringLiteral( "(%1,%2)" ).arg( field.length() ).arg( field.precision() );
    parts << QStringLiteral( "field=%1:%2%3%4" )
               .arg(
                 QString( QUrl::toPercentEncoding( field.name() ) ),
                 memoryLayerFieldType(
                   field.type() == QMetaType::Type::QVariantList
                       || field.type() == QMetaType::Type::QStringList
                     ? field.subType()
                     : field.type(),
                   field.typeName()
                 ),
                 lengthPrecision,
                 field.type() == QMetaType::Type::QVariantList
                     || field.type() == QMetaType::Type::QStringList
                   ? QStringLiteral( "[]" )
                   : QString()
               );
  }

  const QString uri = geomType + '?' + parts.join( '&' );
  QgsVectorLayer::LayerOptions options { QgsCoordinateTransformContext() };
  options.skipCrsValidation = true;
  options.loadDefaultStyle = loadDefaultStyle;

  return std::make_shared<QgsVectorLayer>( uri, name, MemoryProvider::providerKey(), options );
}
