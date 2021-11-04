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
#include <qgsfillsymbollayer.h>
#include <qgsvectorlayerlabeling.h>
#include <qgspallabeling.h>
#include <qgscallout.h>
#include <qgssymbol.h>
#include <QFile>
#include <QString>

#if VERSION_INT >= 31200
#	include <qgsrulebasedlabeling.h>
#endif

namespace SymbolLayerType
{
    static const QString SvgMarker = "SvgMarker";
    static const QString SVGFill = "SVGFill";
};

namespace LabelingType
{
    static const QString RuleBased = "rule-based";
};

const HeadlessRender::Style::Category HeadlessRender::Style::DefaultImportCategories = QgsMapLayer::Symbology | QgsMapLayer::Symbology3D | QgsMapLayer::Labeling | QgsMapLayer::Rendering;

static QSharedPointer<QgsVectorLayer> createTemporaryLayer( const std::string &style )
{
    QDomDocument document;
    QString errorMessage;
    QgsReadWriteContext context;
    document.setContent( QString::fromStdString( style ) );

    QSharedPointer<QgsVectorLayer> qgsVectorLayer( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) ) );
    qgsVectorLayer->readStyle( document.firstChild(), errorMessage, context, static_cast<QgsMapLayer::StyleCategory>( HeadlessRender::Style::DefaultImportCategories )  );
    return qgsVectorLayer;
}

static bool validateStyle( const std::string &style, QString &errorMessage )
{
    QDomDocument document;
    document.setContent( QString::fromStdString( style ), &errorMessage );

    QgsVectorLayer::LayerOptions layerOptions;

    QDomElement myRoot = document.firstChildElement( QStringLiteral( "qgis" ) );
    if ( !myRoot.isNull() )
    {
        switch( static_cast<QgsWkbTypes::GeometryType>( myRoot.firstChildElement( QStringLiteral( "layerGeometryType" ) ).text().toInt() ) )
        {
        case QgsWkbTypes::GeometryType::PointGeometry:
            layerOptions.fallbackWkbType = QgsWkbTypes::Point;
        break;
        case QgsWkbTypes::GeometryType::LineGeometry:
            layerOptions.fallbackWkbType = QgsWkbTypes::LineString;
        break;
        case QgsWkbTypes::GeometryType::PolygonGeometry:
            layerOptions.fallbackWkbType = QgsWkbTypes::Polygon;
        break;
        case QgsWkbTypes::GeometryType::UnknownGeometry:
            layerOptions.fallbackWkbType = QgsWkbTypes::Unknown;
        break;
        case QgsWkbTypes::GeometryType::NullGeometry:
            layerOptions.fallbackWkbType = QgsWkbTypes::NoGeometry;
        break;
        }
    }
    else
    {
        layerOptions.fallbackWkbType = QgsWkbTypes::Point;
    }

    QSharedPointer<QgsVectorLayer> qgsVectorLayer( new QgsVectorLayer( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions ) );
    return qgsVectorLayer->importNamedStyle( document, errorMessage );
}

HeadlessRender::Style HeadlessRender::Style::fromString( const std::string &string, const SvgResolverCallback &svgResolverCallback /* = nullptr */ )
{
    QString errorMessage;
    if ( !validateStyle( string, errorMessage ) )
        throw HeadlessRender::StyleValidationError( errorMessage.toStdString() );

    Style style;
    style.mData = string;

    if (svgResolverCallback)
        style.mData = resolveSvgPaths( style.mData, svgResolverCallback );

    return style;
}

HeadlessRender::Style HeadlessRender::Style::fromFile( const std::string &filePath, const SvgResolverCallback &svgResolverCallback /* = nullptr */ )
{
    std::string data;
    QFile file( QString::fromStdString( filePath) );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QByteArray byteArray = file.readAll();
        data = std::string( byteArray.constData(), byteArray.length() );
    }

    return HeadlessRender::Style::fromString( data, svgResolverCallback );
}

std::string HeadlessRender::Style::data() const
{
    return mData;
}

std::pair<bool, std::set<std::string>> HeadlessRender::Style::usedAttributes() const
{
    std::set<std::string> usedAttributes;

    QSharedPointer<QgsVectorLayer> qgsVectorLayer = createTemporaryLayer( mData );
    QgsRenderContext renderContext;

    QgsAbstractVectorLayerLabeling *abstractVectorLayerLabeling = qgsVectorLayer->labeling();
    if ( abstractVectorLayerLabeling )
    {
        QSet<QString> fields;

        if ( abstractVectorLayerLabeling->type() == LabelingType::RuleBased )
        {
#if VERSION_INT >= 31200
            QgsRuleBasedLabeling *ruleBasedLabeling = dynamic_cast<QgsRuleBasedLabeling *>( abstractVectorLayerLabeling );
            if ( ruleBasedLabeling->rootRule() )
                for ( const QgsRuleBasedLabeling::Rule *rule : ruleBasedLabeling->rootRule()->children() )
                    fields.unite( QgsExpression( rule->filterExpression() ).referencedColumns() );
#else
            return std::make_pair( false, usedAttributes );
#endif
        }

        for ( const QString &providerId : qgsVectorLayer->labeling()->subProviders() )
        {
#if VERSION_INT < 31400
            fields.unite( referencedFields( qgsVectorLayer, renderContext, providerId ));
#else
            fields.unite( qgsVectorLayer->labeling()->settings( providerId ).referencedFields( renderContext ));
#endif
        }

        for ( const QString &field : fields )
            usedAttributes.insert( field.toStdString() );
    }

    for (const QString &attr : qgsVectorLayer->renderer()->usedAttributes( renderContext ))
        usedAttributes.insert( attr.toStdString() );

    return std::make_pair( true, usedAttributes );
}

void HeadlessRender::Style::resolveSymbol( QgsSymbol *symbol, const HeadlessRender::SvgResolverCallback &svgResolverCallback )
{
    for ( QgsSymbolLayer *symbolLayer : symbol->symbolLayers() )
    {
        if ( symbolLayer->layerType() == SymbolLayerType::SvgMarker )
        {
            QgsSvgMarkerSymbolLayer *svgMarkerSymbolLayer = dynamic_cast<QgsSvgMarkerSymbolLayer *>( symbolLayer );
            const std::string &resolvedPath = svgResolverCallback( svgMarkerSymbolLayer->path().toStdString() );
            svgMarkerSymbolLayer->setPath( QString::fromStdString( resolvedPath ) );
        }
        else if ( symbolLayer->layerType() == SymbolLayerType::SVGFill )
        {
            QgsSVGFillSymbolLayer *svgFillSymbolLayer = dynamic_cast<QgsSVGFillSymbolLayer *>( symbolLayer );
            const std::string &resolvedPath = svgResolverCallback( svgFillSymbolLayer->svgFilePath().toStdString() );
            svgFillSymbolLayer->setSvgFilePath( QString::fromStdString( resolvedPath ) );
        }

        if ( symbolLayer->subSymbol() )
            resolveSymbol( symbolLayer->subSymbol(), svgResolverCallback );
    }
}

std::string HeadlessRender::Style::resolveSvgPaths( const std::string &data, const HeadlessRender::SvgResolverCallback &svgResolverCallback)
{
    QDomDocument domDocument;
    QString errorMessage;

    QSharedPointer<QgsVectorLayer> qgsVectorLayer = createTemporaryLayer( data );

    QgsRenderContext renderContext;
    for ( QgsSymbol *symbol : qgsVectorLayer->renderer()->symbols( renderContext ) )
        resolveSymbol( symbol, svgResolverCallback );

    qgsVectorLayer->exportNamedStyle( domDocument, errorMessage );

    return domDocument.toString().toStdString();
}

QSet<QString> HeadlessRender::Style::referencedFields( const QSharedPointer<QgsVectorLayer> &layer, const QgsRenderContext &context, const QString &providerId ) const
{
    QSet<QString> referenced;

    QgsPalLayerSettings settings = layer->labeling()->settings( providerId );

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

HeadlessRender::StyleValidationError::StyleValidationError(const std::string &message)
    : errorMessage( message )
{

}

const char *HeadlessRender::StyleValidationError::what() const throw()
{
    return errorMessage.c_str();
}
