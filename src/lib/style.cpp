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

#if VERSION_INT >= 33000
#	include <qgsdiagramrenderer.h>
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
    static const QString DIAGRAM_CATEGORY = "DiagramCategory";
    static const QString ENABLED = "enabled";
}

const HeadlessRender::Style::Category HeadlessRender::Style::DefaultImportCategories = QgsMapLayer::Symbology
                                                                                     | QgsMapLayer::Symbology3D
                                                                                     | QgsMapLayer::Labeling
                                                                                     | QgsMapLayer::Rendering
                                                                                     | QgsMapLayer::CustomProperties
                                                                                     | QgsMapLayer::Diagrams;

static HeadlessRender::QgsMapLayerPtr createTemporaryVectorLayer( const QgsVectorLayer::LayerOptions &layerOptions )
{
    return HeadlessRender::QgsMapLayerPtr( new QgsVectorLayer( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions ) );
}

static HeadlessRender::QgsMapLayerPtr createTemporaryRasterLayer()
{
    QgsRasterLayer::LayerOptions layerOptions;
    layerOptions.loadDefaultStyle = false;
    return HeadlessRender::QgsMapLayerPtr( new QgsRasterLayer( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions ) );
}

bool HeadlessRender::Style::importToLayer( HeadlessRender::QgsMapLayerPtr &layer, QDomDocument &styleData, QString &errorMessage )
{
    return layer->importNamedStyle( styleData, errorMessage, static_cast<QgsMapLayer::StyleCategory>( HeadlessRender::Style::DefaultImportCategories ) );
}

HeadlessRender::QgsVectorLayerPtr HeadlessRender::Style::createTemporaryVectorLayerWithStyle( QString &errorMessage ) const
{
    if ( !mCachedTemporaryLayer )
    {
        QgsVectorLayer::LayerOptions layerOptions;
        QDomDocument styleData = mData;

        QDomElement myRoot = styleData.firstChildElement( TAGS::QGIS );
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

        HeadlessRender::QgsMapLayerPtr qgsVectorLayer = createTemporaryVectorLayer( layerOptions );
        if ( !Style::importToLayer( qgsVectorLayer, styleData, errorMessage ))
            return nullptr;

        mCachedTemporaryLayer = qgsVectorLayer;
    }
    return std::dynamic_pointer_cast<QgsVectorLayer>( mCachedTemporaryLayer );
}

HeadlessRender::QgsRasterLayerPtr HeadlessRender::Style::createTemporaryRasterLayerWithStyle( QString &errorMessage) const
{
    if ( !mCachedTemporaryLayer )
    {
        HeadlessRender::QgsMapLayerPtr qgsRasterLayer = createTemporaryRasterLayer();

        QDomDocument styleData = mData;
        if ( !Style::importToLayer( qgsRasterLayer, styleData, errorMessage ))
            return nullptr;

        mCachedTemporaryLayer = qgsRasterLayer;
    }
    return std::dynamic_pointer_cast<QgsRasterLayer>( mCachedTemporaryLayer );
}

bool HeadlessRender::Style::validateGeometryType( Layer::GeometryType layerGeometryType ) const
{
    QDomElement geometryTypeElement = mData.firstChildElement( TAGS::QGIS ).firstChildElement( TAGS::LAYER_GEOMETRY_TYPE );
    if ( layerGeometryType == HeadlessRender::Layer::GeometryType::Unknown || geometryTypeElement.isNull() )
        return true;

    QgsVectorLayer::LayerOptions layerOptions;
    layerOptions.fallbackWkbType = layerGeometryTypeToQgsWkbType( layerGeometryType );
    HeadlessRender::QgsVectorLayerPtr qgsVectorLayer = std::dynamic_pointer_cast<QgsVectorLayer>( createTemporaryVectorLayer( layerOptions ));

    QgsWkbTypes::GeometryType importLayerGeometryType = static_cast<QgsWkbTypes::GeometryType>( geometryTypeElement.text().toInt() );

    return ( qgsVectorLayer->geometryType() == importLayerGeometryType ? true : false );
}

bool HeadlessRender::Style::validateStyle( QString &errorMessage ) const
{
    return createTemporaryVectorLayerWithStyle( errorMessage ) ? true : false;
}

HeadlessRender::Style HeadlessRender::Style::fromString( const std::string &data,
                                                         const SvgResolverCallback &svgResolverCallback /* = nullptr */,
                                                         Layer::GeometryType layerGeometryType /* = Layer::GeometryType::Undefined */,
                                                         DataType layerType /* = DataType::Unknown */ )
{
    QDomDocument domDocument;
    domDocument.setContent( QString::fromStdString( data ) );

    Style style;
    style.mData = domDocument;

    if ( !style.validateGeometryType( layerGeometryType ))
        throw StyleTypeMismatch( "Style type mismatch" );

    QString errorMessage;
    if ( !style.validateStyle( errorMessage ))
        throw StyleValidationError( errorMessage );

    if ( layerType != DataType::Unknown && style.type() != layerType )
        throw StyleTypeMismatch( "Layer type and style type do not match" );

    if (svgResolverCallback)
    {
        style.mData = style.resolveSvgPaths( svgResolverCallback );
        style.mCachedTemporaryLayer.reset();
    }

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

QDomDocument HeadlessRender::Style::data() const
{
    return mData;
}

HeadlessRender::UsedAttributes HeadlessRender::Style::readUsedAttributes() const
{
    std::set<std::string> usedAttributes;

    if ( isDefaultStyle() )
        return std::make_pair( true, usedAttributes );

    QString errorMessage;
    HeadlessRender::QgsVectorLayerPtr qgsVectorLayer = createTemporaryVectorLayerWithStyle( errorMessage );
    if ( !qgsVectorLayer )
        throw QgisHeadlessError( errorMessage );

    if ( hasEnabledDiagrams( qgsVectorLayer ))
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

bool HeadlessRender::Style::hasEnabledDiagrams( const QgsVectorLayerPtr &layer ) const
{
    bool enabled = false;

#if VERSION_INT >= 33000
    const QgsDiagramRenderer *diagramRenderer = layer->diagramRenderer();
    if ( diagramRenderer )
        for (const auto &item: diagramRenderer->diagramSettings())
        {
            enabled = item.enabled;
            if (enabled)
                break;
        }
#else
    Q_UNUSED( layer )

    const QDomNodeList nodes = mData.elementsByTagName( TAGS::DIAGRAM_CATEGORY );
    for ( int i = 0; i < nodes.size(); ++i )
    {
        const QDomElement element = nodes.item( i ).toElement();
        const QString enabledStr = element.attribute( TAGS::ENABLED, "0" );

        enabled = enabledStr.toInt();
        if ( enabled )
            break;
    }
#endif

    return enabled;
}

HeadlessRender::UsedAttributes HeadlessRender::Style::usedAttributes() const
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
    if ( mType == HeadlessRender::DataType::Unknown && !mData.isNull() )
    {
        QDomElement root = mData.firstChildElement( TAGS::QGIS );

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
    HeadlessRender::QgsMapLayerPtr qgsMapLayer;
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
            qgsMapLayer = createTemporaryRasterLayerWithStyle( errorMessage );
        else
            qgsMapLayer = createTemporaryVectorLayerWithStyle( errorMessage );

        if ( !qgsMapLayer )
            throw QgisHeadlessError( errorMessage );
    }

    QDomDocument exportedStyle;
    qgsMapLayer->exportNamedStyle( exportedStyle, errorMessage,  QgsReadWriteContext(),  static_cast<QgsMapLayer::StyleCategory>( HeadlessRender::Style::DefaultImportCategories ));

    return exportedStyle.toString();
}

void HeadlessRender::Style::resolveSymbol( QgsSymbol *symbol, const HeadlessRender::SvgResolverCallback &svgResolverCallback ) const
{
    for ( QgsSymbolLayer *symbolLayer : symbol->symbolLayers() )
    {
        if ( symbolLayer->layerType() == SymbolLayerType::SvgMarker )
        {
            QgsSvgMarkerSymbolLayer *svgMarkerSymbolLayer = dynamic_cast<QgsSvgMarkerSymbolLayer *>( symbolLayer );
            const std::string &resolvedPath = svgResolverCallback( svgMarkerSymbolLayer->path().toStdString() );
            
            const QColor fillColor = svgMarkerSymbolLayer->fillColor();
            const QColor strokeColor = svgMarkerSymbolLayer->strokeColor();
            const double strokeWidth = svgMarkerSymbolLayer->strokeWidth();

            svgMarkerSymbolLayer->setPath( QString::fromStdString( resolvedPath ) );
            
            svgMarkerSymbolLayer->setFillColor( fillColor );
            svgMarkerSymbolLayer->setStrokeColor( strokeColor );
            svgMarkerSymbolLayer->setStrokeWidth( strokeWidth );
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

QDomDocument HeadlessRender::Style::resolveSvgPaths( const HeadlessRender::SvgResolverCallback &svgResolverCallback) const
{
    QDomDocument exportedStyle;
    QString errorMessage;

    HeadlessRender::QgsVectorLayerPtr qgsVectorLayer = createTemporaryVectorLayerWithStyle( errorMessage );
    if ( !qgsVectorLayer )
        throw QgisHeadlessError( errorMessage );

    QgsRenderContext renderContext;
    for ( QgsSymbol *symbol : qgsVectorLayer->renderer()->symbols( renderContext ) )
        resolveSymbol( symbol, svgResolverCallback );

    qgsVectorLayer->exportNamedStyle( exportedStyle, errorMessage );
    if ( !errorMessage.isEmpty() )
        throw QgisHeadlessError( errorMessage );

    QDomElement myRoot = mData.firstChildElement( TAGS::QGIS );
    if ( myRoot.isNull() || myRoot.firstChildElement( TAGS::LAYER_GEOMETRY_TYPE ).text().isEmpty())
        removeLayerGeometryTypeElement( exportedStyle );

    return exportedStyle;
}

QSet<QString> HeadlessRender::Style::referencedFields( const HeadlessRender::QgsVectorLayerPtr &layer, const QgsRenderContext &context, const QString &providerId ) const
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

void HeadlessRender::Style::removeLayerGeometryTypeElement( QDomDocument &domDocument ) const
{
    QDomElement root = domDocument.firstChildElement( TAGS::QGIS );
    if ( !root.isNull() )
    {
        QDomElement child = root.firstChildElement( TAGS::LAYER_GEOMETRY_TYPE );
        if ( !child.isNull() )
            root.removeChild( child );
    }
}
