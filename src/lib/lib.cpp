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

#include <QApplication>
#include <QSizeF>
#include <cstdlib>

static QApplication *app = nullptr;

void HeadlessRender::init( int argc, char **argv )
{
    setenv("QT_QPA_PLATFORM", "offscreen", true);

    app = new QgsApplication( argc, argv, false, "", "offscreen" );
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
    mSettings->setFlag( QgsMapSettings::RenderBlocking );
}

void HeadlessRender::MapRequest::setDpi( int dpi )
{
    mSettings->setOutputDpi( dpi );
}

void HeadlessRender::MapRequest::setCrs( const HeadlessRender::CRS &crs )
{
    mSettings->setDestinationCrs( *crs.qgsCoordinateReferenceSystem() );
}

void HeadlessRender::MapRequest::addLayer( const HeadlessRender::Layer &layer, const Style &style, const std::string &label /* = "" */ )
{
    QgsMapLayerPtr qgsMapLayer = layer.qgsMapLayer();
    if ( !qgsMapLayer )
        return;

    QString readStyleError;
    QDomDocument domDocument;
    domDocument.setContent( QString::fromStdString( style.data() ) );
    QgsReadWriteContext context;

    qgsMapLayer->readStyle( domDocument.firstChild(), readStyleError, context, static_cast<QgsMapLayer::StyleCategory>( HeadlessRender::Style::DefaultImportCategories ) );
    qgsMapLayer->setName( QString::fromStdString( label ) );

    mLayers.push_back( qgsMapLayer );

    QList<QgsMapLayer *> qgsMapLayers;
    for ( const QgsMapLayerPtr &layer : mLayers )
        qgsMapLayers.push_back( layer.get() );
    mSettings->setLayers( qgsMapLayers );

    mQgsLayerTree->addLayer( qgsMapLayer.get() );
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

    QEventLoop eventLoop;
    QObject::connect( &job, &QgsMapRendererParallelJob::finished, &eventLoop, &QEventLoop::quit );

    job.start();
    eventLoop.exec();
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
