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
#include <qgsvectorlayer.h>
#include <qgsrenderer.h>
#include <qgsrendercontext.h>
#include <qgsmarkersymbollayer.h>
#include <qgsvectorlayerlabeling.h>
#include <qgspallabeling.h>
#include <qgscallout.h>
#include <QFile>
#include <QString>

const HeadlessRender::Style::Category HeadlessRender::Style::DefaultImportCategories = QgsMapLayer::Symbology | QgsMapLayer::Symbology3D | QgsMapLayer::Labeling;

QSharedPointer<QgsVectorLayer> createTemporaryLayer( const std::string &style )
{
    QDomDocument document;
    QString errorMessage;
    QgsReadWriteContext context;
    document.setContent( QString::fromStdString( style ) );

    QSharedPointer<QgsVectorLayer> qgsVectorLayer( new QgsVectorLayer( QStringLiteral( "Point?field=col1:real" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) ) );
    qgsVectorLayer->readStyle( document.firstChild(), errorMessage, context, static_cast<QgsMapLayer::StyleCategory>( HeadlessRender::Style::DefaultImportCategories )  );
    return qgsVectorLayer;
}

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

std::set<std::string> HeadlessRender::Style::usedAttributes() const
{
    std::set<std::string> usedAttributes;

    QSharedPointer<QgsVectorLayer> qgsVectorLayer = createTemporaryLayer( mData );
    QgsRenderContext renderContext;

    for (const QString &attr : qgsVectorLayer->renderer()->usedAttributes( renderContext ))
        usedAttributes.insert( attr.toStdString() );

    const QSet<QString> &fields = referencedFields( qgsVectorLayer, renderContext );
    for ( const QString &field : fields )
        usedAttributes.insert( field.toStdString() );

    return usedAttributes;
}

std::string HeadlessRender::Style::resolveSvgPaths( const std::string &data, const HeadlessRender::SvgResolverCallback &svgResolverCallback)
{
    QDomDocument domDocument;
    QString errorMessage;

    QSharedPointer<QgsVectorLayer> qgsVectorLayer = createTemporaryLayer( data );

    QgsRenderContext renderContext;
    for ( const auto &symbol : qgsVectorLayer->renderer()->symbols( renderContext ) )
    {
        for ( const auto &symbolLayer : symbol->symbolLayers() )
        {
            if ( symbolLayer->layerType() == "SvgMarker" )
            {
                QgsSvgMarkerSymbolLayer *svgMarkerSymbolLayer = dynamic_cast<QgsSvgMarkerSymbolLayer *>( symbolLayer );
                const std::string &resolvedPath = svgResolverCallback( svgMarkerSymbolLayer->path().toStdString() );
                svgMarkerSymbolLayer->setPath( QString::fromStdString( resolvedPath ) );
            }
        }
    }

    qgsVectorLayer->exportNamedStyle( domDocument, errorMessage );

    return domDocument.toString().toStdString();
}

QSet<QString> HeadlessRender::Style::referencedFields( const QSharedPointer<QgsVectorLayer> &layer, const QgsRenderContext &context ) const
{
    auto settings = layer->labeling()->settings();

    QSet<QString> referenced;
    if ( settings.drawLabels )
    {
        if ( settings.isExpression )
            referenced.unite( QgsExpression( settings.fieldName ).referencedColumns() );
        else
            referenced.insert( settings.fieldName );
    }

    referenced.unite( settings.dataDefinedProperties().referencedFields( context.expressionContext() ) );

    if ( settings.geometryGeneratorEnabled )
    {
        QgsExpression geomGeneratorExpr( settings.geometryGenerator );
        referenced.unite( geomGeneratorExpr.referencedColumns() );
    }

    if ( settings.callout() )
        referenced.unite( settings.callout()->referencedFields( context ) );

    return referenced;
}
