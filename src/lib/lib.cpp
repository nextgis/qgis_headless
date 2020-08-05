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
#include "qgsnetworkaccessmanager.h"
#include "qgsmaprenderersequentialjob.h"

#include <QApplication>
#include <cstdlib>

static QApplication *app = nullptr;

void HeadlessRender::init( int argc, char **argv )
{
    setenv("QT_QPA_PLATFORM", "offscreen", true);

    app = new QApplication( argc, argv );

    QgsNetworkAccessManager::instance();
}

void HeadlessRender::deinit()
{
    delete app;
}

const char * HeadlessRender::getVersion()
{
    return QGIS_HEADLESS_LIB_VERSION_STRING;
}

HeadlessRender::MapRequest::MapRequest()
    : mSettings( new QgsMapSettings )
{
    mSettings->setBackgroundColor( Qt::transparent );
}

void HeadlessRender::MapRequest::setDpi( int dpi )
{
    mSettings->setOutputDpi( dpi );
}

void HeadlessRender::MapRequest::setSvgPaths( const std::vector<std::string> &paths )
{
    // NOT IMPLEMENTED
}

void HeadlessRender::MapRequest::setCrs( const HeadlessRender::CRS &crs )
{
    mSettings->setDestinationCrs( *crs.qgsCoordinateReferenceSystem() );
}

void HeadlessRender::MapRequest::addLayer( HeadlessRender::Layer layer, const Style &style )
{
    QString readStyleError;
    QDomDocument domDocument;
    domDocument.setContent( QString::fromStdString( style.data() ) );
    QgsReadWriteContext context;

    QgsMapLayerPtr qgsMapLayer = layer.qgsMapLayer();
    qgsMapLayer->readStyle( domDocument.firstChild(), readStyleError, context );

    mLayers.push_back( qgsMapLayer );
}

HeadlessRender::ImagePtr HeadlessRender::MapRequest::renderImage( const Extent &extent, const Size &size )
{
    double minx = std::get<0>( extent );
    double miny = std::get<1>( extent );
    double maxx = std::get<2>( extent );
    double maxy = std::get<3>( extent );

    int width = std::get<0>( size );
    int height = std::get<1>( size );

    QList<QgsMapLayer *> qgsMapLayers;
    for ( const QgsMapLayerPtr &layer : mLayers )
        qgsMapLayers.push_back( layer.get() );

    mSettings->setOutputSize( { width, height } );
    mSettings->setLayers( qgsMapLayers );
    mSettings->setExtent( QgsRectangle( minx, miny, maxx, maxy ) );

    QgsMapRendererSequentialJob job( *mSettings );

    job.start();
    job.waitForFinished();

    return imageData( job.renderedImage() );
}

void HeadlessRender::MapRequest::renderLegend( const Size &size )
{
    // NOT IMPLEMENTED
}

HeadlessRender::ImagePtr HeadlessRender::MapRequest::imageData( const QImage &image, int quality )
{
    QByteArray bytes;
    QBuffer buffer( &bytes );

    buffer.open( QIODevice::WriteOnly );
    image.save( &buffer, "PNG", quality );
    buffer.close();

    const int size = bytes.size();
    unsigned char *data = (unsigned char *) malloc( size );
    memcpy( data, reinterpret_cast<unsigned char *>(bytes.data()), size );

    return std::make_shared<HeadlessRender::Image>( data, size );
}
