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

#include "style.h"

#include "layer.h"
#include "qgsvectorlayer.h"
#include "qgsrenderer.h"
#include "qgsrendercontext.h"
#include "qgsmarkersymbollayer.h"
#include <QFile>
#include <QString>

HeadlessRender::Style HeadlessRender::Style::fromString( const std::string &string, const SvgResolverCallback &svgResolverCallback /* = nullptr */ )
{
    Style style;
    style.mData = string;

    if (svgResolverCallback)
        style.mData = resolveSvgPaths( style.mData, svgResolverCallback );

    return style;
}

HeadlessRender::Style HeadlessRender::Style::fromFile( const std::string &filePath, const SvgResolverCallback &svgResolverCallback /* = nullptr */ )
{
    Style style;

    QFile file( QString::fromStdString( filePath) );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QByteArray byteArray = file.readAll();
        style.mData = std::string( byteArray.constData(), byteArray.length() );
    }

    if (svgResolverCallback)
        style.mData = resolveSvgPaths( style.mData, svgResolverCallback );

    return style;
}

std::string HeadlessRender::Style::data() const
{
    return mData;
}

std::string HeadlessRender::Style::resolveSvgPaths( const std::string &data, const HeadlessRender::SvgResolverCallback &svgResolverCallback)
{
    QDomDocument domDocument;
    QString errorMessage;
    QgsReadWriteContext context;

    std::unique_ptr<QgsVectorLayer> qgsVectorLayer = std::unique_ptr<QgsVectorLayer>();
    domDocument.setContent( QString::fromStdString( data ) );
    qgsVectorLayer->readStyle( domDocument.firstChild(), errorMessage, context );

    QString dataWithResolvedPaths = QString::fromStdString( data );

    QgsRenderContext renderContext;
    for ( const auto &symbol : qgsVectorLayer->renderer()->symbols( renderContext ) )
    {
        for ( const auto &symbolLayer : symbol->symbolLayers() )
        {
            if ( symbolLayer->layerType() == "SvgMarker" )
            {
                QgsSvgMarkerSymbolLayer *svgMarkerSymbolLayer = dynamic_cast<QgsSvgMarkerSymbolLayer *>( symbolLayer );
                std::string resolvedPath = svgResolverCallback( svgMarkerSymbolLayer->path().toStdString() );
                dataWithResolvedPaths.replace( svgMarkerSymbolLayer->path(), QString::fromStdString( resolvedPath ) );
            }
        }
    }

    return dataWithResolvedPaths.toStdString();
}
