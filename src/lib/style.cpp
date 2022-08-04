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
#include "utils.h"

#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsrenderer.h>
#include <qgsrendercontext.h>
#include <qgsmarkersymbollayer.h>
#include <qgsfillsymbollayer.h>
#include <qgsvectorlayerlabeling.h>
#include <qgspallabeling.h>
#include <qgscallout.h>
#include <qgssymbol.h>
#include <qgslinesymbol.h>
#include <qgsfillsymbol.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbollayer.h>
#include <qgssinglesymbolrenderer.h>

#include <QFile>
#include <QString>
#include <QTextStream>

#if VERSION_INT >= 31200
#	include <qgsrulebasedlabeling.h>
#endif

namespace SymbolLayerType
{
    static const QString SvgMarker = "SvgMarker";
    static const QString SVGFill = "SVGFill";
}

namespace LabelingType
{
    static const QString RuleBased = "rule-based";
}

namespace TAGS
{
    static const QString QGIS = "qgis";
    static const QString LAYER_GEOMETRY_TYPE = "layerGeometryType";
    static const QString PIPE = "pipe";
    static const QString RASTER_PROPERTIES = "rasterproperties";
    static const QString RASTER_RENDERER = "rasterrenderer";
}

const HeadlessRender::Style::Category HeadlessRender::Style::DefaultImportCategories = QgsMapLayer::Symbology
                                                                                     | QgsMapLayer::Symbology3D
                                                                                     | QgsMapLayer::Labeling
                                                                                     | QgsMapLayer::Rendering
                                                                                     | QgsMapLayer::CustomProperties
                                                                                     | QgsMapLayer::Diagrams;

static QSharedPointer<QgsVectorLayer> createTemporaryVectorLayer( const QgsVectorLayer::LayerOptions &layerOptions )
{
    return QSharedPointer<QgsVectorLayer>( new QgsVectorLayer( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions ) );
}

static QSharedPointer<QgsRasterLayer> createTemporaryRasterLayer()
{
    QgsRasterLayer::LayerOptions layerOptions;
    layerOptions.loadDefaultStyle = false;
    return QSharedPointer<QgsRasterLayer>( new QgsRasterLayer( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions ) );
}


QSharedPointer<QgsVectorLayer> HeadlessRender::Style::createTemporaryVectorLayerWithStyle( const std::string &style, QString &errorMessage )
{
    QDomDocument document;
    document.setContent( QString::fromStdString( style ), &errorMessage );
    return createTemporaryVectorLayerWithStyle( document, errorMessage );
}

QSharedPointer<QgsVectorLayer> HeadlessRender::Style::createTemporaryVectorLayerWithStyle( QDomDocument &style, QString &errorMessage )
{
    QgsVectorLayer::LayerOptions layerOptions;

    QDomElement myRoot = style.firstChildElement( TAGS::QGIS );
    if ( !myRoot.isNull() )
    {
        switch( static_cast<QgsWkbTypes::GeometryType>( myRoot.firstChildElement( TAGS::LAYER_GEOMETRY_TYPE ).text().toInt() ) )
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

    QSharedPointer<QgsVectorLayer> qgsVectorLayer = createTemporaryVectorLayer( layerOptions );
    bool importStyleStatus = qgsVectorLayer->importNamedStyle( style, errorMessage, static_cast<QgsMapLayer::StyleCategory>( HeadlessRender::Style::DefaultImportCategories ) );
    if ( !importStyleStatus )
        return nullptr;

    return qgsVectorLayer;
}

QSharedPointer<QgsRasterLayer> HeadlessRender::Style::createTemporaryRasterLayerWithStyle(const std::string &style, QString &errorMessage)
{
    QSharedPointer<QgsRasterLayer> qgsRasterLayer = createTemporaryRasterLayer();

    QDomDocument styleDomDocument;
    styleDomDocument.setContent( QString::fromStdString( style ), &errorMessage );

    bool importStyleStatus = qgsRasterLayer->importNamedStyle( styleDomDocument, errorMessage, static_cast<QgsMapLayer::StyleCategory>( HeadlessRender::Style::DefaultImportCategories ));
    if ( !importStyleStatus )
        return nullptr;

    return qgsRasterLayer;
}

bool HeadlessRender::Style::validateGeometryType( const std::string &style, Layer::GeometryType layerGeometryType )
{
    QDomDocument styleDomDocument;
    styleDomDocument.setContent( QString::fromStdString( style ) );

    QDomElement geometryTypeElement = styleDomDocument.firstChildElement( TAGS::QGIS ).firstChildElement( TAGS::LAYER_GEOMETRY_TYPE );
    if ( layerGeometryType == HeadlessRender::Layer::GeometryType::Unknown || geometryTypeElement.isNull() )
        return true;

    QgsVectorLayer::LayerOptions layerOptions;
    layerOptions.fallbackWkbType = layerGeometryTypeToQgsWkbType( layerGeometryType );
    QSharedPointer<QgsVectorLayer> qgsVectorLayer = createTemporaryVectorLayer( layerOptions );

    QgsWkbTypes::GeometryType importLayerGeometryType = static_cast<QgsWkbTypes::GeometryType>( geometryTypeElement.text().toInt() );

    return ( qgsVectorLayer->geometryType() == importLayerGeometryType ? true : false );
}

bool HeadlessRender::Style::validateStyle( const std::string &style, QString &errorMessage )
{
    return createTemporaryVectorLayerWithStyle( style, errorMessage ) ? true : false;
}

HeadlessRender::Style HeadlessRender::Style::fromString( const std::string &string,
                                                         const SvgResolverCallback &svgResolverCallback /* = nullptr */,
                                                         Layer::GeometryType layerGeometryType /* = Layer::GeometryType::Undefined */,
                                                         DataType layerType /* = DataType::Unknown */ )
{
    if ( !validateGeometryType( string, layerGeometryType ))
        throw StyleTypeMismatch( "Style type mismatch" );

    QString errorMessage;
    if ( !validateStyle( string, errorMessage ) )
        throw StyleValidationError( errorMessage );

    Style style;
    style.mData = string;

    if ( layerType != DataType::Unknown && style.type() != layerType )
        throw StyleTypeMismatch( "Layer type and style type do not match" );

    if (svgResolverCallback)
        style.mData = resolveSvgPaths( style.mData, svgResolverCallback );

    return style;
}

HeadlessRender::Style HeadlessRender::Style::fromFile( const std::string &filePath,
                                                       const SvgResolverCallback &svgResolverCallback /* = nullptr */,
                                                       Layer::GeometryType layerGeometryType /* = Layer::GeometryType::Unknown */,
                                                       DataType layerType /* = DataType::Unknown */ )
{
    std::string data;
    QFile file( QString::fromStdString( filePath) );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QByteArray byteArray = file.readAll();
        data = std::string( byteArray.constData(), byteArray.length() );
    }

    return HeadlessRender::Style::fromString( data, svgResolverCallback, layerGeometryType, layerType );
}

HeadlessRender::Style HeadlessRender::Style::fromDefaults( const QColor &color,
                                                           Layer::GeometryType layerGeometryType /* = Layer::GeometryType::Undefined */,
                                                           DataType layerType /* = DataType::Unknown */  )
{
    HeadlessRender::Style style;
    style.mDefault = true;
    style.mDefaultStyleParams = { color, layerGeometryType, layerType };
    return style;
}

std::string HeadlessRender::Style::data() const
{
    return mData;
}

HeadlessRender::UsedAttributes HeadlessRender::Style::readUsedAttributes() const
{
    std::set<std::string> usedAttributes;

    if ( isDefaultStyle() )
        return std::make_pair( true, usedAttributes );

    QString errorMessage;
    QSharedPointer<QgsVectorLayer> qgsVectorLayer = createTemporaryVectorLayerWithStyle( mData, errorMessage );
    if ( !qgsVectorLayer )
        throw QgisHeadlessError( errorMessage );

    if ( qgsVectorLayer->diagramRenderer() )
        return std::make_pair( false, usedAttributes );

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

HeadlessRender::UsedAttributes HeadlessRender::Style::usedAttributes()
{
    if (!mUsedAttributesCached)
    {
        mUsedAttributesCache = readUsedAttributes();
        mUsedAttributesCached = true;
    }
    return mUsedAttributesCache;
}

HeadlessRender::DataType HeadlessRender::Style::type() const
{
    if ( mType == HeadlessRender::DataType::Unknown && !mData.empty() )
    {
        QDomDocument styleDomDocument;
        styleDomDocument.setContent( QString::fromStdString( mData ));

        QDomElement root = styleDomDocument.firstChildElement( TAGS::QGIS );

        QDomNode pipeNode = root.firstChildElement( TAGS::PIPE );
        if ( pipeNode.isNull() ) // old project
            pipeNode = root;

        QDomElement rendererElement;
        //rasterlayerproperties element there -> old format (1.8 and early 1.9)
        if ( !root.firstChildElement( TAGS::RASTER_PROPERTIES ).isNull() )
            rendererElement = root.firstChildElement( TAGS::RASTER_RENDERER );
        else
            rendererElement = pipeNode.firstChildElement( TAGS::RASTER_RENDERER );

        mType = rendererElement.isNull() ? HeadlessRender::DataType::Vector : HeadlessRender::DataType::Raster;
    }

    return mType;
}

bool HeadlessRender::Style::isDefaultStyle() const
{
    return mDefault;
}

QColor HeadlessRender::Style::defaultStyleColor() const
{
    return mDefaultStyleParams.color;
}

QString HeadlessRender::Style::exportToQML() const
{
    QSharedPointer<QgsMapLayer> qgsMapLayer;
    QString errorMessage;

    if ( isDefaultStyle() )
    {
        if ( mDefaultStyleParams.layerType == HeadlessRender::DataType::Raster )
        {
            qgsMapLayer = createTemporaryRasterLayer();
        }
        else
        {
            QgsVectorLayer::LayerOptions layerOptions;
            layerOptions.fallbackWkbType = layerGeometryTypeToQgsWkbType( mDefaultStyleParams.layerGeometryType );
            qgsMapLayer = createTemporaryVectorLayer( layerOptions );
        }

        if ( !qgsMapLayer )
            throw QgisHeadlessError( errorMessage );

        if ( mDefaultStyleParams.layerType != HeadlessRender::DataType::Raster )
        {
            QgsSymbol *qgsSymbol;
            QgsSymbolLayer *qgsSymbolLayer;

            QVariantMap props;
            props[QStringLiteral( "color" )] = mDefaultStyleParams.color.name( QColor::NameFormat::HexArgb );

            switch( mDefaultStyleParams.layerGeometryType )
            {
            case HeadlessRender::Layer::GeometryType::Point:
            case HeadlessRender::Layer::GeometryType::PointZ:
            case HeadlessRender::Layer::GeometryType::MultiPoint:
            case HeadlessRender::Layer::GeometryType::MultiPointZ:
                qgsSymbol = new QgsMarkerSymbol;
                qgsSymbolLayer = QgsSimpleMarkerSymbolLayer::create( props );
                break;
            case HeadlessRender::Layer::GeometryType::LineString:
            case HeadlessRender::Layer::GeometryType::LineStringZ:
            case HeadlessRender::Layer::GeometryType::MultiLineString:
            case HeadlessRender::Layer::GeometryType::MultiLineStringZ:
                qgsSymbol = new QgsLineSymbol;
                qgsSymbolLayer = QgsSimpleLineSymbolLayer::create( props );
                break;
            case HeadlessRender::Layer::GeometryType::Polygon:
            case HeadlessRender::Layer::GeometryType::PolygonZ:
            case HeadlessRender::Layer::GeometryType::MultiPolygon:
            case HeadlessRender::Layer::GeometryType::MultiPolygonZ:
                qgsSymbol = new QgsFillSymbol;
                qgsSymbolLayer = QgsSimpleFillSymbolLayer::create( props );
                break;
            case HeadlessRender::Layer::GeometryType::Unknown:
                qgsSymbol = nullptr;
                qgsSymbolLayer = nullptr;
                break;
            }

            if ( qgsSymbol && qgsSymbolLayer )
            {
                qgsSymbol->changeSymbolLayer( 0, qgsSymbolLayer );
                QgsSingleSymbolRenderer *qgsSingleSymbolRenderer = new QgsSingleSymbolRenderer( qgsSymbol );
                static_cast<QgsVectorLayer *>( qgsMapLayer.get() )->setRenderer( qgsSingleSymbolRenderer );
            }
        }
    }
    else
    {
        if ( mDefaultStyleParams.layerType == HeadlessRender::DataType::Raster )
            qgsMapLayer = createTemporaryRasterLayerWithStyle( mData, errorMessage );
        else
            qgsMapLayer = createTemporaryVectorLayerWithStyle( mData, errorMessage );

        if ( !qgsMapLayer )
            throw QgisHeadlessError( errorMessage );
    }

    QDomDocument exportedStyle;
    qgsMapLayer->exportNamedStyle( exportedStyle, errorMessage );

    return exportedStyle.toString();
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
    QDomDocument style;
    QDomDocument exportedStyle;
    QString errorMessage;

    style.setContent( QString::fromStdString( data ), &errorMessage );
    if ( !errorMessage.isEmpty() )
        throw QgisHeadlessError( errorMessage );

    QSharedPointer<QgsVectorLayer> qgsVectorLayer = createTemporaryVectorLayerWithStyle( style, errorMessage );
    if ( !qgsVectorLayer )
        throw QgisHeadlessError( errorMessage );

    QgsRenderContext renderContext;
    for ( QgsSymbol *symbol : qgsVectorLayer->renderer()->symbols( renderContext ) )
        resolveSymbol( symbol, svgResolverCallback );

    qgsVectorLayer->exportNamedStyle( exportedStyle, errorMessage );
    if ( !errorMessage.isEmpty() )
        throw QgisHeadlessError( errorMessage );

    QDomElement myRoot = style.firstChildElement( TAGS::QGIS );
    if ( myRoot.isNull() || myRoot.firstChildElement( TAGS::LAYER_GEOMETRY_TYPE ).text().isEmpty())
        removeLayerGeometryTypeElement( exportedStyle );

    return exportedStyle.toString().toStdString();
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

void HeadlessRender::Style::removeLayerGeometryTypeElement( QDomDocument &domDocument )
{
    QDomElement root = domDocument.firstChildElement( TAGS::QGIS );
    if ( !root.isNull() )
    {
        QDomElement child = root.firstChildElement( TAGS::LAYER_GEOMETRY_TYPE );
        if ( !child.isNull() )
            root.removeChild( child );
    }
}
