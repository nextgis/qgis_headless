/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*  Author:  Denis Ilyin, denis.ilyin@nextgis.com
*******************************************************************************
*  Copyright (C) 2020 NextGIS, info@nextgis.ru
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "lib.h"

#include "version.h"
#include <qgsnetworkaccessmanager.h>
#include <qgsmaprendererparalleljob.h>
#include <qgslegendrenderer.h>
#include <qgslegendsettings.h>
#include <qgslayertreemodel.h>
#include <qgslayertree.h>
#include <qgsrendercontext.h>
#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include <qgslayoutexporter.h>
#include <qgsmaprenderercustompainterjob.h>

#include "exceptions.h"

#include <QApplication>
#include <QSizeF>
#include <QPrinter>
#include <QJsonArray>
#include <cstdlib>

static QApplication *app = nullptr;
static HeadlessRender::LogLevel appLogLevel = HeadlessRender::LogLevel::Debug;

static void messageHandler( QtMsgType msgType, const QMessageLogContext &, const QString &msg )
{
    const QByteArray &logMessage = msg.toLocal8Bit();

    switch ( appLogLevel )
    {
    case HeadlessRender::LogLevel::Debug:
        if ( msgType == QtDebugMsg )
            qDebug() << logMessage;
        // fall down
    case HeadlessRender::LogLevel::Info:
        if ( msgType == QtInfoMsg )
            qInfo() << logMessage;
        // fall down
    case HeadlessRender::LogLevel::Warning:
        if ( msgType == QtWarningMsg )
            qWarning() << logMessage;
        // fall down
    case HeadlessRender::LogLevel::Critical:
        if ( msgType == QtCriticalMsg )
            qCritical() << logMessage;
        break;
    }

    if ( msgType == QtFatalMsg )
        qFatal( "%s", logMessage.constData() );
}

void HeadlessRender::init( int argc, char **argv )
{
    qInstallMessageHandler( messageHandler );

    QByteArray platform( "offscreen" );
    qputenv( "QT_QPA_PLATFORM", platform );

    app = new QgsApplication( argc, argv, false, "", platform );
    QgsApplication::initQgis();
}

void HeadlessRender::deinit()
{
    QgsApplication::exitQgis();
    delete app;
}

void HeadlessRender::setSvgPaths( const std::vector<std::string> &paths )
{
    QStringList svgPaths;
    for (const std::string &path : paths)
        svgPaths.push_back( QString::fromStdString( path ) );
    QgsApplication::instance()->setDefaultSvgPaths( svgPaths );
}

std::vector<std::string> HeadlessRender::getSvgPaths()
{
    std::vector<std::string> svgPaths;
    for ( const QString &path : QgsApplication::instance()->svgPaths() )
        svgPaths.push_back( path.toStdString() );
    return svgPaths;
}

const char * HeadlessRender::getVersion()
{
    return QGIS_HEADLESS_LIB_VERSION_STRING;
}

const char * HeadlessRender::getQGISVersion()
{
    return VERSION;
}

HeadlessRender::MapRequest::MapRequest()
    : mSettings( new QgsMapSettings )
    , mQgsLayerTree( new QgsLayerTree )
{
    mSettings->setBackgroundColor( Qt::transparent );
#if VERSION_INT > 32200
    mSettings->setFlag( Qgis::MapSettingsFlag::RenderBlocking );
#else
    mSettings->setFlag( QgsMapSettings::RenderBlocking );
#endif
}

void HeadlessRender::MapRequest::setDpi( int dpi )
{
    mSettings->setOutputDpi( dpi );
}

void HeadlessRender::MapRequest::setCrs( const HeadlessRender::CRS &crs )
{
    mSettings->setDestinationCrs( *crs.qgsCoordinateReferenceSystem() );
}

int HeadlessRender::MapRequest::addLayer( const HeadlessRender::Layer &layer, const Style &style, const std::string &label /* = "" */ )
{
    QgsMapLayerPtr qgsMapLayer = layer.qgsMapLayer();
    if ( !qgsMapLayer )
        throw QgisHeadlessError( "Layer is null" );

    if ( layer.type() != style.type() )
        throw StyleTypeMismatch( "Layer type and style type do not match" );

    QString readStyleError;
    QDomDocument domDocument;
    domDocument.setContent( QString::fromStdString( style.data() ) );
    QgsReadWriteContext context;

    bool importStyleStatus = qgsMapLayer->importNamedStyle( domDocument, readStyleError, static_cast<QgsMapLayer::StyleCategory>( HeadlessRender::Style::DefaultImportCategories ) );

    if ( !importStyleStatus )
        throw QgisHeadlessError( readStyleError );

    qgsMapLayer->setName( QString::fromStdString( label ) );

    mLayers.push_back( qgsMapLayer );

    QList<QgsMapLayer *> qgsMapLayers;
    for ( const QgsMapLayerPtr &layer : mLayers )
        qgsMapLayers.push_back( layer.get() );
    mSettings->setLayers( qgsMapLayers );

    mQgsLayerTree->addLayer( qgsMapLayer.get() );

    int addedLayerIndex = qgsMapLayers.size() - 1;
    return addedLayerIndex;
}

HeadlessRender::ImagePtr HeadlessRender::MapRequest::renderImage( const Extent &extent, const Size &size )
{
    double minx = std::get<0>( extent );
    double miny = std::get<1>( extent );
    double maxx = std::get<2>( extent );
    double maxy = std::get<3>( extent );

    int width = std::get<0>( size );
    int height = std::get<1>( size );

    mSettings->setOutputSize( { width, height } );
    mSettings->setExtent( QgsRectangle( minx, miny, maxx, maxy ) );

    QgsMapRendererParallelJob job( *mSettings );

    job.start();
    job.waitForFinished();

    return std::make_shared<HeadlessRender::Image>( job.renderedImage() );
}

HeadlessRender::ImagePtr HeadlessRender::MapRequest::renderLegend( const Size &size /* = Size() */ )
{
    int width = std::get<0>( size );
    int height = std::get<1>( size );

    QgsLayerTreeModel legendModel( mQgsLayerTree.get() );
    QgsLegendRenderer legendRenderer( &legendModel, QgsLegendSettings() );

    int dpi = mSettings->outputDpi();
    qreal dpmm = dpi / 25.4;
    QImage img;

    if ( !width || !height )
    {
        QSizeF minSize = legendRenderer.minimumSize();
        img = QImage( QSize( minSize.width() * dpmm, minSize.height() * dpmm ), QImage::Format_ARGB32_Premultiplied );
    }
    else
    {
        img = QImage( width, height, QImage::Format_ARGB32_Premultiplied );
    }

    img.fill( Qt::transparent );

    QPainter painter( &img );
    painter.setRenderHint( QPainter::Antialiasing, true );
    QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );

    context.painter()->scale( dpmm, dpmm );

    legendRenderer.drawLegend( context );
    painter.end();

    return std::make_shared<HeadlessRender::Image>( img );
}

void HeadlessRender::MapRequest::exportPdf( const std::string &filepath, const Extent &extent, const HeadlessRender::Size &size)
{
    double minx = std::get<0>( extent );
    double miny = std::get<1>( extent );
    double maxx = std::get<2>( extent );
    double maxy = std::get<3>( extent );

    int width = std::get<0>( size );
    int height = std::get<1>( size );

    mSettings->setOutputSize({ width, height });
    mSettings->setExtent( QgsRectangle( minx, miny, maxx, maxy ) );

    QPrinter printer;
    printer.setOutputFileName( QString::fromStdString( filepath ));
    printer.setOutputFormat( QPrinter::PdfFormat );
    printer.setOrientation( QPrinter::Portrait );

    printer.setPaperSize( mSettings->outputSize() * 25.4 / mSettings->outputDpi(), QPrinter::Millimeter );
    printer.setPageMargins( 0, 0, 0, 0, QPrinter::Millimeter );
    printer.setResolution( mSettings->outputDpi() );

    QPainter painter( &printer );

    QgsMapRendererCustomPainterJob job( *mSettings, &painter );
    job.prepare();
    job.renderPrepared();

    painter.end();
}

std::vector<HeadlessRender::LegendSymbol> HeadlessRender::MapRequest::legendSymbols( size_t index, const HeadlessRender::Size &size /* = Size() */ )
{
    if ( mLayers.size() <= index )
        throw QgisHeadlessError( "Invalid layer index" );

    QgsMapLayerPtr layer = mLayers.at( index );

    int width = std::get<0>( size );
    int height = std::get<1>( size );

    QgsLegendSettings legendSettings;
    if ( width && height )
        legendSettings.setSymbolSize( QSize( width, height ));

    QgsLayerTree qgsLayerTree;
    qgsLayerTree.addLayer( layer.get() );

    QgsLayerTreeModel legendModel( &qgsLayerTree );
    QgsLegendRenderer legendRenderer( &legendModel, legendSettings );

#if VERSION_INT > 31600
    QJsonObject json = legendRenderer.exportLegendToJson( QgsRenderContext() );
#else
    QJsonObject json;
    legendRenderer.exportLegendToJson( QgsRenderContext(), json );
#endif

    std::vector<HeadlessRender::LegendSymbol> legendSymbols;
    QJsonArray nodes = json.value( "nodes" ).toArray();
    processLegendSymbols( nodes, legendSymbols );
    return legendSymbols;
}

void HeadlessRender::MapRequest::processLegendSymbols(QJsonArray nodes, std::vector<HeadlessRender::LegendSymbol> &legendSymbols)
{
    for ( const auto &item : nodes)
    {
        QJsonObject node = item.toObject();

        QString type = node.value( "type" ).toString();
        if ( type == "layer" )
        {
            QJsonArray symbols = node.value( "symbols" ).toArray();
            for ( const auto &symbolItem : symbols)
            {
                QJsonObject symbol = symbolItem.toObject();
                QString iconBase64 = symbol.value( "icon" ).toString();
                QString title = symbol.value( "title" ).toString();

                QImage image = QImage::fromData( QByteArray::fromBase64( iconBase64.toUtf8() ));

                legendSymbols.emplace_back( std::make_shared<Image>( image ), title );
            }
        }
        else if ( type == "group" )
        {
            QJsonArray nodes = node.value( "nodes" ).toArray();
            processLegendSymbols( nodes, legendSymbols );
        }
    }
}

void HeadlessRender::setLoggingLevel( HeadlessRender::LogLevel level )
{
    appLogLevel = level;
}
