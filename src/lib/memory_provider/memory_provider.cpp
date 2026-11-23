/***************************************************************************
    Adapted from QGIS src/core/providers/memory/qgsmemoryprovider.cpp
    copyright            : (C) 2008 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "memory_provider.h"

#include <algorithm>
#include <limits>
#include <utility>

#include <QIcon>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

#include <qgsapplication.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsfeature.h>
#include <qgsfields.h>
#include <qgsgeometry.h>
#include <qgslogger.h>
#include <qgsspatialindex.h>

#include <qgsvariantutils.h>

#include "memory_feature_iterator.h"

using namespace HeadlessRender;

static const auto TextProviderKey = QStringLiteral( "qgis-headless-memory" );
static const auto TextProviderLib = QStringLiteral( "qgis-headless" );
static const auto TextProviderDescription = QStringLiteral( "QGIS Headless memory provider" );

MemoryProvider::MemoryProvider(
  const QString &uri, const ProviderOptions &options, Qgis::DataProviderReadFlags flags
)
  : QgsVectorDataProvider( uri, options, flags )
{
  // Initialize the geometry with the uri to support old style uri's
  // (ie, just 'point', 'line', 'polygon')
  const QUrl url = QUrl::fromEncoded( uri.toUtf8() );
  const QUrlQuery query( url );
  QString geometry;
  if ( query.hasQueryItem( QStringLiteral( "geometry" ) ) )
  {
    geometry = query.queryItemValue( QStringLiteral( "geometry" ) );
  }
  else
  {
    geometry = url.path();
  }

  if ( geometry.compare( QLatin1String( "none" ), Qt::CaseInsensitive ) == 0 )
  {
    mWkbType = Qgis::WkbType::NoGeometry;
  }
  else
  {
    mWkbType = QgsWkbTypes::parseType( geometry );
  }

  if ( query.hasQueryItem( QStringLiteral( "crs" ) ) )
  {
    const QString crsDef = query.queryItemValue( QStringLiteral( "crs" ) );
    mCrs.createFromString( crsDef );
  }
  else
  {
    // TODO - remove in QGIS 4.0. Layers without an explicit CRS set SHOULD have an invalid CRS. But in order to maintain
    // 3.x api, we have to be tolerant/shortsighted(?) here and fallback to EPSG:4326
    mCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
  }

  mLastFeatureId = 0;

  setNativeTypes(
    QList< NativeType >()
    << QgsVectorDataProvider::
         NativeType( tr( "Whole Number (integer)" ), QStringLiteral( "integer" ), QMetaType::Type::Int, 0, 10 )
    // Decimal number from OGR/Shapefile/dbf may come with length up to 32 and
    // precision up to length-2 = 30 (default, if width is not specified in dbf is length = 24 precision = 15)
    // We know that double (QVariant::Double) has only 15-16 significant numbers,
    // but setting that correct limits would disable the use of memory provider with
    // data from Shapefiles. In any case, the data are handled as doubles.
    // So the limits set here are not correct but enable use of data from Shapefiles.
    << QgsVectorDataProvider::
         NativeType( tr( "Decimal Number (real)" ), QStringLiteral( "double" ), QMetaType::Type::Double, 0, 32, 0, 30 )
    << QgsVectorDataProvider::
         NativeType( tr( "Text (string)" ), QStringLiteral( "string" ), QMetaType::Type::QString, 0, 255 )

    // integer types
    << QgsVectorDataProvider::
         NativeType( tr( "Whole Number (smallint - 16bit)" ), QStringLiteral( "int2" ), QMetaType::Type::Int, -1, -1, 0, 0 )
    << QgsVectorDataProvider::
         NativeType( tr( "Whole Number (integer - 32bit)" ), QStringLiteral( "int4" ), QMetaType::Type::Int, -1, -1, 0, 0 )
    << QgsVectorDataProvider::
         NativeType( tr( "Whole Number (integer - 64bit)" ), QStringLiteral( "int8" ), QMetaType::Type::LongLong, -1, -1, 0, 0 )
    << QgsVectorDataProvider::
         NativeType( tr( "Decimal Number (numeric)" ), QStringLiteral( "numeric" ), QMetaType::Type::Double, 1, 20, 0, 20 )
    << QgsVectorDataProvider::
         NativeType( tr( "Decimal Number (decimal)" ), QStringLiteral( "decimal" ), QMetaType::Type::Double, 1, 20, 0, 20 )

    // floating point
    << QgsVectorDataProvider::
         NativeType( tr( "Decimal Number (real)" ), QStringLiteral( "real" ), QMetaType::Type::Double, -1, -1, -1, -1 )
    << QgsVectorDataProvider::
         NativeType( tr( "Decimal Number (double)" ), QStringLiteral( "double precision" ), QMetaType::Type::Double, -1, -1, -1, -1 )

    // string types
    << QgsVectorDataProvider::
         NativeType( tr( "Text, unlimited length (text)" ), QStringLiteral( "text" ), QMetaType::Type::QString, -1, -1, -1, -1 )

    // date type
    << QgsVectorDataProvider::NativeType(
         QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), QStringLiteral( "date" ),
         QMetaType::Type::QDate, -1, -1, -1, -1
       )
    << QgsVectorDataProvider::NativeType(
         QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), QStringLiteral( "time" ),
         QMetaType::Type::QTime, -1, -1, -1, -1
       )
    << QgsVectorDataProvider::NativeType(
         QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ),
         QStringLiteral( "datetime" ), QMetaType::Type::QDateTime, -1, -1, -1, -1
       )

    // boolean
    << QgsVectorDataProvider::
         NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Bool ), QStringLiteral( "boolean" ), QMetaType::Type::Bool )

    // blob
    << QgsVectorDataProvider::
         NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QByteArray ), QStringLiteral( "binary" ), QMetaType::Type::QByteArray )

    // list types
    << QgsVectorDataProvider::NativeType(
         QgsVariantUtils::typeToDisplayString( QMetaType::Type::QStringList ),
         QStringLiteral( "stringlist" ), QMetaType::Type::QStringList, 0, 0, 0, 0,
         QMetaType::Type::QString
       )
    << QgsVectorDataProvider::NativeType(
         QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::Int ),
         QStringLiteral( "integerlist" ), QMetaType::Type::QVariantList, 0, 0, 0, 0,
         QMetaType::Type::Int
       )
    << QgsVectorDataProvider::NativeType(
         QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::Double ),
         QStringLiteral( "doublelist" ), QMetaType::Type::QVariantList, 0, 0, 0, 0,
         QMetaType::Type::Double
       )
    << QgsVectorDataProvider::NativeType(
         QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::LongLong ),
         QStringLiteral( "integer64list" ), QMetaType::Type::QVariantList, 0, 0, 0, 0,
         QMetaType::Type::LongLong
       )

    // complex types
    << QgsVectorDataProvider::NativeType(
         QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantMap ),
         QStringLiteral( "map" ), QMetaType::Type::QVariantMap, -1, -1, -1, -1
       )
    << QgsVectorDataProvider::NativeType( tr( "Geometry" ), QStringLiteral( "geometry" ), QMetaType::Type::User )
  );

  if ( query.hasQueryItem( QStringLiteral( "field" ) ) )
  {
    QList<QgsField> attributes;
    const thread_local QRegularExpression reFieldDef(
      "\\:"
      "([\\w\\s]+)"        // type
      "(?:\\((\\-?\\d+)"   // length
      "(?:\\,(\\-?\\d+))?" // precision
      "\\))?(\\[\\])?"     // array
      "$",
      QRegularExpression::CaseInsensitiveOption
    );
    const QStringList fields = query.allQueryItemValues( QStringLiteral( "field" ) );
    for ( int i = 0; i < fields.size(); i++ )
    {
      QString name = QUrl::fromPercentEncoding( fields.at( i ).toUtf8() );
      const QRegularExpressionMatch regularExpressionMatch = reFieldDef.match( name );

      // If no match -> use string as type
      QMetaType::Type type = QMetaType::Type::QString;
      QMetaType::Type subType = QMetaType::Type::UnknownType;

      QString typeName( QStringLiteral( "string" ) );
      int length = 255;
      int precision = 0;

      if ( regularExpressionMatch.hasMatch() )
      {
        name = name.mid( 0, regularExpressionMatch.capturedStart() );
        typeName = regularExpressionMatch.captured( 1 ).toLower();

        // Search typeName correspondence in native types
        bool isNativeType = false;
        const QList<QgsVectorDataProvider::NativeType> nativeTypesList( nativeTypes() );
        for ( const NativeType &nativeType : nativeTypesList )
        {
          if ( nativeType.mTypeName.toLower() == typeName )
          {
            isNativeType = true;
            type = nativeType.mType;
            subType = nativeType.mSubType;
            typeName = nativeType.mTypeName;
            break;
          }
        }

        // Not a native type -> check other supported types:
        if ( isNativeType == false )
        {
          if ( typeName == QLatin1String( "int" ) )
          {
            type = QMetaType::Type::Int;
            typeName = QStringLiteral( "integer" );
          }
          else if ( typeName == QLatin1String( "long" ) )
          {
            type = QMetaType::Type::LongLong;
            typeName = QStringLiteral( "int8" );
          }
          else if ( typeName == QLatin1String( "bool" ) )
          {
            type = QMetaType::Type::Bool;
            typeName = QStringLiteral( "boolean" );
          }
          else
          {
            QgsLogger::warning(
              tr( "Unsupported typeName '%1'. Will be handled as string." ).arg( typeName )
            );
            type = QMetaType::Type::QString;
            typeName = QStringLiteral( "string" );
          }
        }

        // Set default length/precision for double/real
        if ( typeName == QLatin1String( "real" ) || typeName == QLatin1String( "double" ) )
        {
          length = 20;
          precision = 5;
        }

        if ( !regularExpressionMatch.captured( 2 ).isEmpty() )
          length = regularExpressionMatch.captured( 2 ).toInt();

        if ( !regularExpressionMatch.captured( 3 ).isEmpty() )
          precision = regularExpressionMatch.captured( 3 ).toInt();

        // Array
        if ( !regularExpressionMatch.captured( 4 ).isEmpty() )
        {
          if ( subType == QMetaType::Type::UnknownType )
            subType = type;

          if ( type != QMetaType::Type::QVariantList && type != QMetaType::Type::QStringList )
            type = type == QMetaType::Type::QString ? QMetaType::Type::QStringList
                                                    : QMetaType::Type::QVariantList;

          const QLatin1String listSuffix( "list" );
          if ( !typeName.endsWith( listSuffix ) )
            typeName += QLatin1String( "list" );
        }
      }

      attributes.append( QgsField( name, type, typeName, length, precision, QString(), subType ) );
    }
    addAttributes( attributes );
  }

  if ( query.hasQueryItem( QStringLiteral( "index" ) )
       && query.queryItemValue( QStringLiteral( "index" ) ) == QLatin1String( "yes" ) )
  {
    createSpatialIndex();
  }
}

MemoryProvider::~MemoryProvider() = default;

QString MemoryProvider::providerKey()
{
  return TextProviderKey;
}

QString MemoryProvider::providerDescription()
{
  return TextProviderDescription;
}

QString MemoryProvider::providerLib()
{
  return TextProviderLib;
}

QgsAbstractFeatureSource *MemoryProvider::featureSource() const
{
  return new MemoryFeatureSource( this );
}

QString MemoryProvider::dataSourceUri( bool expandAuthConfig ) const
{
  Q_UNUSED( expandAuthConfig )

  //QUrl uri( QStringLiteral( "memory" ) );
  QUrl uri( TextProviderKey );
  QUrlQuery query;
  const QString geometry = QgsWkbTypes::displayString( mWkbType );
  query.addQueryItem( QStringLiteral( "geometry" ), geometry );

  if ( mCrs.isValid() )
  {
    QString crsDef;
    const QString authid = mCrs.authid();
    if ( authid.startsWith( QLatin1String( "EPSG:" ) ) )
    {
      crsDef = authid;
    }
    else
    {
      crsDef = QStringLiteral( "wkt:%1" ).arg( mCrs.toWkt( Qgis::CrsWktVariant::Preferred ) );
    }
    query.addQueryItem( QStringLiteral( "crs" ), crsDef );
  }
  if ( mSpatialIndex )
  {
    query.addQueryItem( QStringLiteral( "index" ), QStringLiteral( "yes" ) );
  }

  QgsAttributeList attrs = const_cast<MemoryProvider *>( this )->attributeIndexes();
  for ( int i = 0; i < attrs.size(); i++ )
  {
    const QgsField field = mFields.at( attrs[i] );
    QString fieldDef = field.name();

    QString typeName = field.typeName();
    bool isList = false;
    if ( field.type() == QMetaType::Type::QVariantList
         || field.type() == QMetaType::Type::QStringList )
    {
      switch ( field.subType() )
      {
        case QMetaType::Type::Int:
          typeName = QStringLiteral( "integer" );
          break;

        case QMetaType::Type::LongLong:
          typeName = QStringLiteral( "long" );
          break;

        case QMetaType::Type::Double:
          typeName = QStringLiteral( "double" );
          break;

        case QMetaType::Type::QString:
          typeName = QStringLiteral( "string" );
          break;

        default:
          break;
      }
      isList = true;
    }

    fieldDef.append( QStringLiteral( ":%2(%3,%4)%5" )
                       .arg( typeName )
                       .arg( field.length() )
                       .arg( field.precision() )
                       .arg( isList ? QStringLiteral( "[]" ) : QString() ) );
    query.addQueryItem( QStringLiteral( "field" ), fieldDef );
  }
  uri.setQuery( query );

  return QString( uri.toEncoded() );
}

QString MemoryProvider::storageType() const
{
  return QStringLiteral( "Memory storage" );
}

QgsFeatureIterator MemoryProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator(
    new MemoryFeatureIterator( new MemoryFeatureSource( this ), true, request )
  );
}


QgsRectangle MemoryProvider::extent() const
{
  if ( mExtent.isEmpty() && !mFeatures.isEmpty() )
  {
    mExtent.setNull();

    if ( mSubsetString.isEmpty() )
    {
      // fast way - iterate through all features
      const auto constMFeatures = mFeatures;
      for ( const QgsFeature &feat : constMFeatures )
      {
        if ( feat.hasGeometry() )
          mExtent.combineExtentWith( feat.geometry().boundingBox() );
      }
    }
    else
    {
      QgsFeature f;
      QgsFeatureIterator fi = getFeatures( QgsFeatureRequest().setNoAttributes() );
      while ( fi.nextFeature( f ) )
      {
        if ( f.hasGeometry() )
          mExtent.combineExtentWith( f.geometry().boundingBox() );
      }
    }
  }
  else if ( mFeatures.isEmpty() )
  {
    mExtent.setNull();
  }

  return mExtent;
}

Qgis::WkbType MemoryProvider::wkbType() const
{
  return mWkbType;
}

long long MemoryProvider::featureCount() const
{
  if ( mSubsetString.isEmpty() )
    return mFeatures.count();

  // subset string set, no alternative but testing each feature
  QgsFeatureIterator fit = QgsFeatureIterator(
    new MemoryFeatureIterator( new MemoryFeatureSource( this ), true, QgsFeatureRequest().setNoAttributes() )
  );
  long long count = 0;
  QgsFeature feature;
  while ( fit.nextFeature( feature ) )
  {
    count++;
  }
  return count;
}

QgsFields MemoryProvider::fields() const
{
  return mFields;
}

bool MemoryProvider::isValid() const
{
  return ( mWkbType != Qgis::WkbType::Unknown );
}

QgsCoordinateReferenceSystem MemoryProvider::crs() const
{
  // TODO: make provider projection-aware
  return mCrs; // return default CRS
}

void MemoryProvider::handlePostCloneOperations( QgsVectorDataProvider *source )
{
  if ( MemoryProvider *other = qobject_cast< MemoryProvider * >( source ) )
  {
    // these properties aren't copied when cloning a memory provider by uri, so we need to do it manually
    mFeatures = other->mFeatures;
    mFeatureOrder = other->mFeatureOrder;
    mLastFeatureId = other->mLastFeatureId;
    mExtent = other->mExtent;
    mSpatialIndex = other->mSpatialIndex ? std::make_unique<QgsSpatialIndex>( *other->mSpatialIndex )
                                         : nullptr;
  }
}

bool MemoryProvider::insertFeature( QgsFeature &feature, bool updateExtent )
{
  feature.setValid( true );
  const int fieldCount = mFields.count();
  const int attributeCount = feature.attributeCount();
  if ( attributeCount < fieldCount )
  {
    QgsAttributes attributes = feature.attributes();
    for ( int i = attributeCount; i < fieldCount; ++i )
      attributes.append( QgsVariantUtils::createNullVariant( mFields.at( i ).type() ) );
    feature.setAttributes( attributes );
  }
  else if ( attributeCount > fieldCount )
  {
    pushError( tr( "Feature has too many attributes (expecting %1, received %2)" )
                 .arg( fieldCount )
                 .arg( attributeCount ) );
    QgsAttributes attributes = feature.attributes();
    attributes.resize( fieldCount );
    feature.setAttributes( attributes );
  }

  if ( feature.hasGeometry() && mWkbType == Qgis::WkbType::NoGeometry )
  {
    feature.clearGeometry();
  }
  else if ( feature.hasGeometry()
            && QgsWkbTypes::geometryType( feature.geometry().wkbType() )
                 != QgsWkbTypes::geometryType( mWkbType ) )
  {
    pushError(
      tr( "Could not add feature with geometry type %1 to layer of type %2" )
        .arg( QgsWkbTypes::displayString( feature.geometry().wkbType() ), QgsWkbTypes::displayString( mWkbType ) )
    );
    return false;
  }

  bool conversionError = false;
  QString errorMessage;
  for ( int i = 0; i < fieldCount; ++i )
  {
    QVariant attrValue = feature.attribute( i );
    if ( QgsVariantUtils::isNull( attrValue ) )
      continue;

    if ( !mFields.at( i ).convertCompatible( attrValue, &errorMessage ) )
    {
      if ( !conversionError )
      {
        pushError(
          tr( "Could not store attribute \"%1\": %2" ).arg( mFields.at( i ).name(), errorMessage )
        );
      }
      conversionError = true;
      continue;
    }

    // Conversion can also change the value without changing its type (e.g. rounding).
    feature.setAttribute( i, attrValue );
  }
  if ( conversionError )
    return false;

  QgsFeatureId featureId = feature.id();
  if ( featureId < 0 )
  {
    if ( mLastFeatureId == std::numeric_limits<QgsFeatureId>::max() )
    {
      pushError( tr( "Cannot allocate a feature ID: maximum ID has been reached" ) );
      return false;
    }
    featureId = mLastFeatureId + 1;
  }

  if ( mFeatures.contains( featureId ) )
  {
    pushError( tr( "Feature with ID %1 already exists" ).arg( featureId ) );
    return false;
  }

  feature.setId( featureId );
  mFeatures.insert( featureId, feature );
  mFeatureOrder.append( featureId );
  mLastFeatureId = std::max( mLastFeatureId, featureId );

  if ( feature.hasGeometry() )
  {
    if ( updateExtent )
      mExtent.combineExtentWith( feature.geometry().boundingBox() );
    if ( mSpatialIndex )
      mSpatialIndex->addFeature( feature );
  }
  return true;
}

bool MemoryProvider::addFeature( QgsFeature &feature, Flags flags )
{
  const bool updateExtent = mFeatures.isEmpty() || !mExtent.isEmpty();
  // FastInsert previously updated only the sink's temporary copy.
  bool result;
  if ( flags.testFlag( QgsFeatureSink::FastInsert ) )
  {
    QgsFeature copy = feature;
    result = insertFeature( copy, updateExtent );
  }
  else
  {
    result = insertFeature( feature, updateExtent );
  }
  if ( result || !flags.testFlag( QgsFeatureSink::RollBackOnErrors ) )
    clearMinMaxCache();
  return result;
}

bool MemoryProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  const bool updateExtent = mFeatures.isEmpty() || !mExtent.isEmpty();
  const auto oldExtent { mExtent };
  const auto oldLastFeatureId { mLastFeatureId };
  const auto oldOrderSize { mFeatureOrder.size() };

  bool result = true;
  for ( auto &feature : flist )
  {
    if ( !insertFeature( feature, updateExtent ) )
    {
      result = false;
      break;
    }
  }

  // Roll back
  if ( !result && flags.testFlag( QgsFeatureSink::Flag::RollBackOnErrors ) )
  {
    while ( mFeatureOrder.size() > oldOrderSize )
    {
      const auto addedFid = mFeatureOrder.takeLast();
      if ( mSpatialIndex )
        mSpatialIndex->deleteFeature( mFeatures.value( addedFid ) );
      mFeatures.remove( addedFid );
    }
    mExtent = oldExtent;
    mLastFeatureId = oldLastFeatureId;
  }
  else
  {
    clearMinMaxCache();
  }

  return result;
}

bool MemoryProvider::deleteFeatures( const QgsFeatureIds &id )
{
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    const auto fit = mFeatures.find( *it );

    // check whether such feature exists
    if ( fit == mFeatures.end() )
      continue;

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->deleteFeature( *fit );

    mFeatures.erase( fit );
  }
  mFeatureOrder.erase(
    std::remove_if(
      mFeatureOrder.begin(), mFeatureOrder.end(),
      [&id]( QgsFeatureId fid ) { return id.contains( fid ); }
    ),
    mFeatureOrder.end()
  );

  updateExtents();
  clearMinMaxCache();

  return true;
}

bool MemoryProvider::addAttributes( const QList<QgsField> &attributes )
{
  bool fieldWasAdded { false };
  for ( QgsField field : attributes )
  {
    if ( !supportedType( field ) )
      continue;

    // Make sure added attributes typeName correspond to a native type name
    bool isNativeTypeName = false;

    NativeType nativeTypeCandidate( QString(), QString(), QMetaType::Type::UnknownType );

    const QList<QgsVectorDataProvider::NativeType> nativeTypesList( nativeTypes() );
    for ( const NativeType &nativeType : nativeTypesList )
    {
      if ( nativeType.mTypeName.toLower() == field.typeName().toLower() )
      {
        isNativeTypeName = true;
        break;
      }

      if ( nativeType.mType == field.type()
           && nativeTypeCandidate.mType == QMetaType::Type::UnknownType )
        nativeTypeCandidate = nativeType;
    }
    if ( !isNativeTypeName )
    {
      if ( nativeTypeCandidate.mType == QMetaType::Type::UnknownType )
      {
        QgsLogger::warning( "Field type not supported: " + field.typeName() );
        continue;
      }

      field.setTypeName( nativeTypeCandidate.mTypeName );
    }

    // add new field as a last one
    mFields.append( field );
    fieldWasAdded = true;

    for ( QgsFeature &f : mFeatures )
    {
      QgsAttributes attr = f.attributes();
      attr.append( QVariant() );
      f.setAttributes( attr );
    }
  }
  return fieldWasAdded;
}

bool MemoryProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
{
  QgsFieldNameMap::const_iterator renameIt = renamedAttributes.constBegin();
  bool result = true;
  for ( ; renameIt != renamedAttributes.constEnd(); ++renameIt )
  {
    const int fieldIndex = renameIt.key();
    if ( fieldIndex < 0 || fieldIndex >= mFields.count() )
    {
      result = false;
      continue;
    }
    if ( mFields.indexFromName( renameIt.value() ) >= 0 )
    {
      //field name already in use
      result = false;
      continue;
    }

    mFields.rename( fieldIndex, renameIt.value() );
  }
  return result;
}

bool MemoryProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  QList<int> attrIdx( attributes.begin(), attributes.end() );
  std::sort( attrIdx.begin(), attrIdx.end(), std::greater<int>() );

  // delete attributes one-by-one with decreasing index
  for ( QList<int>::const_iterator it = attrIdx.constBegin(); it != attrIdx.constEnd(); ++it )
  {
    const int idx = *it;
    mFields.remove( idx );

    for ( QgsFeature &f : mFeatures )
    {
      QgsAttributes attr = f.attributes();
      attr.remove( idx );
      f.setAttributes( attr );
    }
  }
  clearMinMaxCache();
  return true;
}

bool MemoryProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  bool result { true };

  QgsChangedAttributesMap rollBackMap;

  QString errorMessage;
  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    const auto fit = mFeatures.find( it.key() );
    if ( fit == mFeatures.end() )
      continue;

    const QgsAttributeMap &attrs = it.value();
    QgsAttributeMap rollBackAttrs;

    // Break on errors
    for ( QgsAttributeMap::const_iterator it2 = attrs.constBegin(); it2 != attrs.constEnd(); ++it2 )
    {
      const int fieldIndex = it2.key();
      if ( fieldIndex < 0 || fieldIndex >= mFields.count() )
        continue;

      QVariant attrValue = it2.value();
      if ( attrValue.userType() == qMetaTypeId< QgsUnsetAttributeValue >() )
        continue;

      // Check attribute conversion
      const bool conversionError {
        !QgsVariantUtils::isNull( attrValue )
        && !mFields.at( it2.key() ).convertCompatible( attrValue, &errorMessage )
      };
      if ( conversionError )
      {
        // Push first conversion error only
        if ( result )
        {
          pushError( tr( "Could not change attribute %1 having type %2 for feature %4: %3" )
                       .arg( mFields.at( it2.key() ).name(), it2.value().typeName(), errorMessage )
                       .arg( it.key() ) );
        }
        result = false;
        break;
      }
      rollBackAttrs.insert( it2.key(), fit->attribute( it2.key() ) );
      fit->setAttribute( it2.key(), attrValue );
    }
    rollBackMap.insert( it.key(), rollBackAttrs );
  }

  // Roll back
  if ( !result )
  {
    changeAttributeValues( rollBackMap );
  }
  else
  {
    clearMinMaxCache();
  }
  return result;
}

bool MemoryProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  for ( QgsGeometryMap::const_iterator it = geometry_map.begin(); it != geometry_map.end(); ++it )
  {
    const auto fit = mFeatures.find( it.key() );
    if ( fit == mFeatures.end() )
      continue;

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->deleteFeature( *fit );

    fit->setGeometry( it.value() );

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->addFeature( *fit );
  }

  updateExtents();

  return true;
}

QString MemoryProvider::subsetString() const
{
  return mSubsetString;
}

bool MemoryProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  Q_UNUSED( updateFeatureCount )

  if ( !theSQL.isEmpty() )
  {
    const QgsExpression tempExpression( theSQL );
    if ( tempExpression.hasParserError() )
      return false;
  }

  if ( theSQL == mSubsetString )
    return true;

  mSubsetString = theSQL;
  clearMinMaxCache();
  mExtent.setNull();

  emit dataChanged();
  return true;
}

bool MemoryProvider::supportsSubsetString() const
{
  return true;
}

QString MemoryProvider::subsetStringDialect() const
{
  return tr( "QGIS expression" );
}

QString MemoryProvider::subsetStringHelpUrl() const
{
  // unfortunately we can't access QgsHelp here, that's a GUI class!
  return QString();
}

Qgis::SpatialIndexPresence MemoryProvider::hasSpatialIndex() const
{
  return mSpatialIndex ? Qgis::SpatialIndexPresence::Present
                       : Qgis::SpatialIndexPresence::NotPresent;
}

bool MemoryProvider::createSpatialIndex()
{
  if ( !mSpatialIndex )
  {
    mSpatialIndex = std::make_unique<QgsSpatialIndex>();

    // Index traversal can depend on insertion order. Keep the previous FID order.
    auto orderedIds = mFeatureOrder;
    std::sort( orderedIds.begin(), orderedIds.end() );
    for ( const auto fid : std::as_const( orderedIds ) )
    {
      QgsFeature feature = mFeatures.constFind( fid ).value();
      mSpatialIndex->addFeature( feature );
    }
  }
  return true;
}

MemoryProvider::Capabilities MemoryProvider::capabilities() const
{
  Capabilities caps {
    Qgis::VectorProviderCapability::AddFeatures | Qgis::VectorProviderCapability::DeleteFeatures
    | Qgis::VectorProviderCapability::ChangeAttributeValues
    | Qgis::VectorProviderCapability::AddAttributes | Qgis::VectorProviderCapability::DeleteAttributes
    | Qgis::VectorProviderCapability::RenameAttributes | Qgis::VectorProviderCapability::SelectAtId
    | Qgis::VectorProviderCapability::FastTruncate
  };
  if ( mWkbType != Qgis::WkbType::NoGeometry )
  {
    caps |= Qgis::VectorProviderCapability::CreateSpatialIndex
            | Qgis::VectorProviderCapability::CircularGeometries
            | Qgis::VectorProviderCapability::ChangeGeometries;
  }
  return caps;
}

bool MemoryProvider::truncate()
{
  if ( mSpatialIndex )
    mSpatialIndex = std::make_unique<QgsSpatialIndex>();
  mFeatures.clear();
  mFeatureOrder.clear();
  clearMinMaxCache();
  mExtent.setNull();
  return true;
}

void MemoryProvider::updateExtents()
{
  mExtent.setNull();
}

QString MemoryProvider::name() const
{
  return TextProviderKey;
}

QString MemoryProvider::description() const
{
  return TextProviderDescription;
}
