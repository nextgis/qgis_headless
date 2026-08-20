#include <limits>
#include <memory>
#include <stdexcept>

#include <QDir>
#include <QTemporaryDir>
#include <QtTest>

#include <qgsfeaturerequest.h>
#include <qgsgeometry.h>
#include <qgsproviderregistry.h>
#include <qgssettings.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

#include "lib.h"

namespace
{
  std::unique_ptr<QgsVectorDataProvider> createProvider(
    const QString &uri = QStringLiteral( "Point?crs=EPSG:3857&field=value:integer" )
  )
  {
    std::unique_ptr<QgsDataProvider> provider(
      QgsProviderRegistry::instance()
        ->createProvider( QStringLiteral( "qgis-headless-memory" ), uri, QgsDataProvider::ProviderOptions() )
    );
    auto *vectorProvider = qobject_cast<QgsVectorDataProvider *>( provider.get() );
    if ( !vectorProvider )
      throw std::runtime_error( "Could not create the headless memory provider" );
    provider.release();
    return std::unique_ptr<QgsVectorDataProvider>( vectorProvider );
  }

  QgsFeature createFeature(
    const QgsVectorDataProvider &provider, QgsFeatureId id = FID_NULL,
    const QString &wkt = QStringLiteral( "Point(0 0)" ), const QVariant &value = 42
  )
  {
    QgsFeature feature( provider.fields(), id );
    QgsAttributes attributes( 1 );
    attributes[0] = value;
    feature.setAttributes( attributes );
    feature.setGeometry( QgsGeometry::fromWkt( wkt ) );
    return feature;
  }

  QList<QgsFeatureId> featureIds( QgsFeatureIterator iterator )
  {
    QList<QgsFeatureId> ids;
    QgsFeature feature;
    while ( iterator.nextFeature( feature ) )
      ids.append( feature.id() );
    return ids;
  }

  QgsFeatureRequest spatialRequest()
  {
    return QgsFeatureRequest().setFilterRect( QgsRectangle( -1, -1, 1, 1 ) );
  }
} //namespace

class TestMemoryProvider : public QObject
{
    Q_OBJECT

  private slots:
    void preservesFeatureIds_data()
    {
      QTest::addColumn<qlonglong>( "id" );
      QTest::newRow( "zero" ) << qlonglong( 0 );
      QTest::newRow( "one" ) << qlonglong( 1 );
      QTest::newRow( "positive" ) << qlonglong( 50 );
      QTest::newRow( "maximum" ) << std::numeric_limits<qlonglong>::max();
    }

    void preservesFeatureIds()
    {
      QFETCH( qlonglong, id );
      auto provider = createProvider();
      auto feature = createFeature( *provider, id );
      QVERIFY( provider->addFeature( feature ) );
      QCOMPARE( feature.id(), id );
      QCOMPARE( featureIds( provider->getFeatures( QgsFeatureRequest() ) ), QList<QgsFeatureId> { id } );
    }

    void generatesFeatureIds()
    {
      auto provider = createProvider();
      auto first = createFeature( *provider );
      auto second = createFeature( *provider, -1 );
      QVERIFY( provider->addFeature( first ) );
      QVERIFY( provider->addFeature( second ) );
      QCOMPARE( first.id(), QgsFeatureId( 1 ) );
      QCOMPARE( second.id(), QgsFeatureId( 2 ) );

      auto zero = createFeature( *provider, 0 );
      auto high = createFeature( *provider, 50 );
      auto low = createFeature( *provider, 8 );
      auto automatic = createFeature( *provider );
      QVERIFY( provider->addFeature( zero ) );
      QVERIFY( provider->addFeature( high ) );
      QVERIFY( provider->addFeature( low ) );
      QVERIFY( provider->addFeature( automatic ) );
      QCOMPARE( zero.id(), QgsFeatureId( 0 ) );
      QCOMPARE( automatic.id(), QgsFeatureId( 51 ) );
    }

    void scansInInsertionOrder()
    {
      auto provider = createProvider();
      auto first = createFeature( *provider, 42 );
      auto second = createFeature( *provider, 8 );
      QVERIFY( provider->addFeature( first ) );
      QVERIFY( provider->addFeature( second ) );
      std::unique_ptr<QgsAbstractFeatureSource> snapshot( provider->featureSource() );
      const QList<QgsFeatureId> original { 42, 8 };
      QCOMPARE( featureIds( provider->getFeatures( QgsFeatureRequest() ) ), original );
      QCOMPARE( featureIds( provider->getFeatures( QgsFeatureRequest( 8 ) ) ), QList<QgsFeatureId> { 8 } );
      QVERIFY( provider->deleteFeatures( { 42 } ) );
      QVERIFY( provider->addFeature( first ) );
      const QList<QgsFeatureId> updated { 8, 42 };
      QCOMPARE( featureIds( provider->getFeatures( QgsFeatureRequest() ) ), updated );
      QCOMPARE( featureIds( snapshot->getFeatures( QgsFeatureRequest() ) ), original );
    }

    void reportsDuplicateIds_data()
    {
      preservesFeatureIds_data();
    }

    void reportsDuplicateIds()
    {
      QFETCH( qlonglong, id );
      auto provider = createProvider();
      auto first = createFeature( *provider, id );
      auto duplicate = createFeature( *provider, id );
      QVERIFY( provider->addFeature( first ) );
      QVERIFY( provider->errors().isEmpty() );
      QVERIFY( !provider->addFeature( duplicate ) );
      QCOMPARE( provider->featureCount(), 1LL );
      QVERIFY( !provider->errors().isEmpty() );
      QVERIFY( provider->errors().join( "; " ).contains( QString::number( id ) ) );
    }

    void keepsAllocatorAfterFailure_data()
    {
      QTest::addColumn<bool>( "wrongGeometry" );
      QTest::addColumn<qlonglong>( "id" );
      QTest::newRow( "explicit-geometry" ) << true << qlonglong( 1000 );
      QTest::newRow( "automatic-geometry" ) << true << qlonglong( -1 );
      QTest::newRow( "explicit-attribute" ) << false << qlonglong( 1000 );
      QTest::newRow( "automatic-attribute" ) << false << qlonglong( -1 );
    }

    void keepsAllocatorAfterFailure()
    {
      QFETCH( bool, wrongGeometry );
      QFETCH( qlonglong, id );
      auto provider = createProvider();
      auto invalid
        = createFeature( *provider, id, wrongGeometry ? "LineString(0 0, 1 1)" : "Point(0 0)", wrongGeometry ? QVariant( 42 ) : QVariant( "not an integer" ) );
      QVERIFY( !provider->addFeature( invalid ) );
      auto valid = createFeature( *provider );
      QVERIFY( provider->addFeature( valid ) );
      QCOMPARE( valid.id(), QgsFeatureId( 1 ) );
      QCOMPARE( provider->featureCount(), 1LL );
    }

    void rollsBackBatch_data()
    {
      QTest::addColumn<bool>( "withIndex" );
      QTest::addColumn<bool>( "duplicateId" );
      QTest::newRow( "geometry" ) << false << false;
      QTest::newRow( "geometry-indexed" ) << true << false;
      QTest::newRow( "collision" ) << false << true;
      QTest::newRow( "collision-indexed" ) << true << true;
    }

    void rollsBackBatch()
    {
      QFETCH( bool, withIndex );
      QFETCH( bool, duplicateId );
      auto provider = createProvider();
      if ( withIndex )
        QVERIFY( provider->createSpatialIndex() );
      auto existing = createFeature( *provider, 10, "Point(-10 -10)", 7 );
      QVERIFY( provider->addFeature( existing ) );
      const auto oldExtent = provider->extent();
      QCOMPARE( provider->minimumValue( 0 ).toInt(), 7 );
      QgsFeatureList
        batch { createFeature( *provider, 20, "Point(0 0)", 9 ), createFeature( *provider, duplicateId ? 10 : 1000, duplicateId ? "Point(0 0)" : "LineString(0 0, 1 1)" ) };
      QVERIFY( !provider->addFeatures( batch, QgsFeatureSink::RollBackOnErrors ) );
      QCOMPARE( provider->featureCount(), 1LL );
      QCOMPARE( featureIds( provider->getFeatures( QgsFeatureRequest() ) ), QList<QgsFeatureId> { 10 } );
      QVERIFY( featureIds( provider->getFeatures( spatialRequest() ) ).isEmpty() );
      QVERIFY( provider->extent() == oldExtent );
      QCOMPARE( provider->minimumValue( 0 ).toInt(), 7 );
      auto automatic = createFeature( *provider );
      QVERIFY( provider->addFeature( automatic ) );
      QCOMPARE( automatic.id(), QgsFeatureId( 11 ) );
    }

    void preservesSuccessfulPartialBatch()
    {
      auto provider = createProvider();
      QVERIFY( provider->createSpatialIndex() );
      QgsFeatureList
        batch { createFeature( *provider, 8 ), createFeature( *provider, 1000, "LineString(0 0, 1 1)" ) };
      QVERIFY( !provider->addFeatures( batch ) );
      QCOMPARE( provider->featureCount(), 1LL );
      QCOMPARE( featureIds( provider->getFeatures( spatialRequest() ) ), QList<QgsFeatureId> { 8 } );
      auto automatic = createFeature( *provider );
      QVERIFY( provider->addFeature( automatic ) );
      QCOMPARE( automatic.id(), QgsFeatureId( 9 ) );
    }

    void truncatesSpatialIndex()
    {
      auto provider = createProvider();
      QVERIFY( provider->createSpatialIndex() );
      auto original = createFeature( *provider, 8 );
      QVERIFY( provider->addFeature( original ) );
      std::unique_ptr<QgsAbstractFeatureSource> snapshot( provider->featureSource() );
      QCOMPARE( provider->minimumValue( 0 ).toInt(), 42 );
      QVERIFY( provider->truncate() );
      QCOMPARE( provider->featureCount(), 0LL );
      QCOMPARE( provider->hasSpatialIndex(), Qgis::SpatialIndexPresence::Present );
      QVERIFY( featureIds( provider->getFeatures( spatialRequest() ) ).isEmpty() );
      QVERIFY( provider->extent().isNull() );
      QCOMPARE( featureIds( snapshot->getFeatures( spatialRequest() ) ), QList<QgsFeatureId> { 8 } );
      auto replacement = createFeature( *provider, FID_NULL, "Point(10 10)" );
      QVERIFY( provider->addFeature( replacement ) );
      QCOMPARE( replacement.id(), QgsFeatureId( 9 ) );
      QVERIFY( featureIds( provider->getFeatures( spatialRequest() ) ).isEmpty() );
      QCOMPARE( featureIds( provider->getFeatures( QgsFeatureRequest() ) ), QList<QgsFeatureId> { 9 } );
    }

    void guardsFeatureIdOverflow()
    {
      const auto maximum = std::numeric_limits<QgsFeatureId>::max();
      auto provider = createProvider();
      auto explicitId = createFeature( *provider, maximum - 1 );
      QVERIFY( provider->addFeature( explicitId ) );
      auto last = createFeature( *provider );
      QVERIFY( provider->addFeature( last ) );
      QCOMPARE( last.id(), maximum );
      auto exhausted = createFeature( *provider );
      QVERIFY( !provider->addFeature( exhausted ) );
      QCOMPARE( provider->featureCount(), 2LL );
      QVERIFY( !provider->errors().isEmpty() );
      auto zero = createFeature( *provider, 0 );
      QVERIFY( provider->addFeature( zero ) );
      QCOMPARE( zero.id(), QgsFeatureId( 0 ) );
    }

    void keepsAllocatorAfterRejectedMaximumId()
    {
      auto provider = createProvider();
      auto invalid
        = createFeature( *provider, std::numeric_limits<QgsFeatureId>::max(), "LineString(0 0, 1 1)" );
      QVERIFY( !provider->addFeature( invalid ) );
      auto automatic = createFeature( *provider );
      QVERIFY( provider->addFeature( automatic ) );
      QCOMPARE( automatic.id(), QgsFeatureId( 1 ) );
    }

    void rollsBackExhaustedIds()
    {
      auto provider = createProvider();
      QVERIFY( provider->createSpatialIndex() );
      auto existing = createFeature( *provider, 8, "Point(-10 -10)" );
      QVERIFY( provider->addFeature( existing ) );
      QgsFeatureList
        batch { createFeature( *provider, std::numeric_limits<QgsFeatureId>::max() ), createFeature( *provider ) };
      QVERIFY( !provider->addFeatures( batch, QgsFeatureSink::RollBackOnErrors ) );
      QCOMPARE( provider->featureCount(), 1LL );
      QVERIFY( featureIds( provider->getFeatures( spatialRequest() ) ).isEmpty() );
      auto automatic = createFeature( *provider );
      QVERIFY( provider->addFeature( automatic ) );
      QCOMPARE( automatic.id(), QgsFeatureId( 9 ) );
    }

    void clonesFeatures_data()
    {
      QTest::addColumn<bool>( "withIndex" );
      QTest::newRow( "without-index" ) << false;
      QTest::newRow( "with-index" ) << true;
    }

    // Adapted from QGIS test_provider_memory.py::testCloneFeatures.
    void clonesFeatures()
    {
      QFETCH( bool, withIndex );
      QgsVectorLayer
        layer( QStringLiteral( "Point?crs=EPSG:3857&field=value:integer" ), QStringLiteral( "test" ), QStringLiteral( "qgis-headless-memory" ) );
      QVERIFY( layer.isValid() );
      auto *provider = layer.dataProvider();
      if ( withIndex )
        QVERIFY( provider->createSpatialIndex() );
      auto inside = createFeature( *provider, 42 );
      auto outside = createFeature( *provider, 8, QStringLiteral( "Point(10 10)" ) );
      QVERIFY( provider->addFeature( inside ) );
      QVERIFY( provider->addFeature( outside ) );

      std::unique_ptr<QgsVectorLayer> clone( layer.clone() );
      QVERIFY( clone->isValid() );
      auto *clonedProvider = clone->dataProvider();
      QCOMPARE( clonedProvider->hasSpatialIndex(), provider->hasSpatialIndex() );
      const QList<QgsFeatureId> ordered { 42, 8 };
      QCOMPARE( featureIds( clonedProvider->getFeatures( QgsFeatureRequest() ) ), ordered );
      const QList<QgsFeatureId> expected { 42 };
      QCOMPARE( featureIds( clonedProvider->getFeatures( spatialRequest() ) ), expected );
      QVERIFY( provider->deleteFeatures( { 42 } ) );
      QVERIFY( featureIds( provider->getFeatures( spatialRequest() ) ).isEmpty() );
      QCOMPARE( featureIds( clonedProvider->getFeatures( spatialRequest() ) ), expected );
    }

    void combinesFidAndSpatialFilter_data()
    {
      clonesFeatures_data();
    }

    // Extends the FID request checks in QGIS providertestbase.py with a bounding box.
    void combinesFidAndSpatialFilter()
    {
      QFETCH( bool, withIndex );
      auto provider = createProvider();
      if ( withIndex )
        QVERIFY( provider->createSpatialIndex() );
      QgsFeatureList
        features { createFeature( *provider, 1 ), createFeature( *provider, 2 ), createFeature( *provider, 3, QStringLiteral( "Point(10 10)" ) ) };
      QVERIFY( provider->addFeatures( features ) );

      auto request = spatialRequest();
      QCOMPARE( featureIds( provider->getFeatures( request.setFilterFid( 2 ) ) ), QList<QgsFeatureId> { 2 } );
      QVERIFY( featureIds( provider->getFeatures( request.setFilterFid( 3 ) ) ).isEmpty() );
      QVERIFY( featureIds( provider->getFeatures( request.setFilterFid( 4 ) ) ).isEmpty() );
    }

    // Adapted from QGIS test_provider_memory.py::testAddChangeFeatureConvertAttribute.
    void storesConvertedAttributeOfSameType()
    {
      auto provider = createProvider( QStringLiteral( "Point?crs=EPSG:3857&field=value:double(10,2)" )
      );
      auto single = createFeature( *provider, 1, QStringLiteral( "Point(0 0)" ), 1.234 );
      QVERIFY( provider->addFeature( single ) );
      QgsFeatureList batch { createFeature( *provider, 2, QStringLiteral( "Point(0 0)" ), 1.234 ) };
      QVERIFY( provider->addFeatures( batch ) );

      QgsFeature stored;
      QVERIFY( provider->getFeatures( QgsFeatureRequest( 1 ) ).nextFeature( stored ) );
      QCOMPARE( stored.attribute( 0 ), QVariant( 1.23 ) );
      QVERIFY( provider->getFeatures( QgsFeatureRequest( 2 ) ).nextFeature( stored ) );
      QCOMPARE( stored.attribute( 0 ), QVariant( 1.23 ) );
      QVERIFY( provider->changeAttributeValues( { { 2, { { 0, 2.346 } } } } ) );
      QVERIFY( provider->getFeatures( QgsFeatureRequest( 2 ) ).nextFeature( stored ) );
      QCOMPARE( stored.attribute( 0 ), QVariant( 2.35 ) );
    }
};

int main( int argc, char **argv )
{
  QTemporaryDir profile;
  const QString authDirectory = profile.path() + "/auth";
  if ( !profile.isValid() || !QDir().mkpath( authDirectory ) )
    return 1;
  qputenv( "HOME", profile.path().toUtf8() );
  qputenv( "XDG_CONFIG_HOME", ( profile.path() + "/config" ).toUtf8() );
  qputenv( "XDG_DATA_HOME", ( profile.path() + "/data" ).toUtf8() );
  qputenv( "QGIS_AUTH_DB_DIR_PATH", authDirectory.toUtf8() );
  qputenv( "QGIS_CUSTOM_CONFIG_PATH", profile.path().toUtf8() );
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setApplicationName( "QGIS3" );
  QSettings::setDefaultFormat( QSettings::IniFormat );
  QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, profile.path() + "/config" );
  {
    QgsSettings settings;
    settings.setValue( "use_password_helper", false, QgsSettings::Section::Auth );
    settings.sync();
  }
  HeadlessRender::init( argc, argv );
  TestMemoryProvider test;
  const int result = QTest::qExec( &test, argc, argv );
  HeadlessRender::deinit();
  return result;
}

#include "test_memory_provider.moc"
