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

#include "exceptions.h"
#include "utils.h"

#include <qgsrenderer.h>
#include <qgsrendercontext.h>
#include <qgsmarkersymbollayer.h>
#include <qgsfillsymbollayer.h>
#include <qgsrasterlayer.h>
#include <qgsvectorlayerlabeling.h>
#include <qgspallabeling.h>
#include <qgscallout.h>
#include <qgslinesymbol.h>
#include <qgsfillsymbol.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbollayer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsmaplayerstylemanager.h>
#include <qgsrulebasedlabeling.h>
#include <QFile>

#if VERSION_INT >= 33000
#	include <qgsdiagramrenderer.h>
#endif

namespace HeadlessRender
{
    namespace TAGS
    {
        const QString QGIS = "qgis";
        const QString LAYER_GEOMETRY_TYPE = "layerGeometryType";
        const QString PIPE = "pipe";
        const QString RASTER_PROPERTIES = "rasterproperties";
        const QString RASTER_RENDERER = "rasterrenderer";
        const QString DIAGRAM_CATEGORY = "DiagramCategory";
        const QString ENABLED = "enabled";
        const QString STYLED_LAYER_DESCRIPTOR = "StyledLayerDescriptor";
        const QString NAMED_LAYER = "NamedLayer";
    }

    namespace SymbolLayerType
    {
        const QString SvgMarker = "SvgMarker";
        const QString SVGFill = "SVGFill";
    }

    namespace LabelingType
    {
        const QString RuleBased = "rule-based";
    }

    namespace ErrorString
    {
        const QString StyleMismatch = "Style type mismatch";
        const QString LayerStyleMismatch = "Layer type and style type do not match";
        const QString AddStyleFailed = "AddStyle failed";
    }
}

using namespace HeadlessRender;

const StyleCategory Style::DefaultImportCategories = QgsMapLayer::Symbology
        | QgsMapLayer::Symbology3D
        | QgsMapLayer::Labeling
        | QgsMapLayer::Rendering
        | QgsMapLayer::CustomProperties
        | QgsMapLayer::Diagrams;

static QgsMapLayerPtr createTemporaryRasterLayer()
{
    QgsRasterLayer::LayerOptions layerOptions;
    layerOptions.loadDefaultStyle = false;
    return HeadlessRender::QgsMapLayerPtr( new QgsRasterLayer( QStringLiteral( "" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ), layerOptions ) );
}


HeadlessRender::Style HeadlessRender::Style::fromString( const std::string &data,
                                                        const SvgResolverCallback &svgResolverCallback /* = nullptr */,
                                                        LayerGeometryType layerGeometryType /* = LayerGeometryType::Undefined */,
                                                        DataType layerType /* = DataType::Unknown */,
                                                        StyleFormat format /* = StyleFormat::QML */)
{
    Style style;
    switch( format )
    {
    case StyleFormat::QML:
        style.init({
               QString::fromStdString( data ),
               svgResolverCallback,
               layerGeometryType,
               layerType
           });
        break;
    case StyleFormat::SLD:

        QDomDocument styleDom;
        styleDom.setContent( QString::fromStdString(data), true );

        const QDomElement myRoot = styleDom.firstChildElement( TAGS::STYLED_LAYER_DESCRIPTOR );
        const QDomElement namedLayerElem = myRoot.firstChildElement( TAGS::NAMED_LAYER );

        QgsVectorLayer::LayerOptions layerOptions;
        if ( namedLayerElem.elementsByTagName("PolygonSymbolizer").size() != 0)
            layerOptions.fallbackWkbType = QgsWkbTypes::Polygon;
        else if ( namedLayerElem.elementsByTagName("LineSymbolizer").size() != 0)
            layerOptions.fallbackWkbType = QgsWkbTypes::LineString;
        else
            layerOptions.fallbackWkbType = QgsWkbTypes::Point;
        HeadlessRender::QgsMapLayerPtr layer = createTemporaryVectorLayer( layerOptions );

        QString errorMessage;
        if (layer->readSld( namedLayerElem, errorMessage ))
        {
            layer->exportNamedStyle( styleDom, errorMessage,  {},  static_cast<QgsMapLayer::StyleCategory>( DefaultImportCategories ));
            style.init({
                   styleDom.toString(),
                   svgResolverCallback,
                   layerGeometryType,
                   layerType
               });
        }
        else
        {
            throw StyleValidationError( "Cannot import SLD style" );
        }

        break;
    }

    return style;
}

HeadlessRender::Style HeadlessRender::Style::fromFile( const std::string &filePath,
                                                       const SvgResolverCallback &svgResolverCallback /* = nullptr */,
                                                       LayerGeometryType layerGeometryType /* = LayerGeometryType::Unknown */,
                                                       DataType layerType /* = DataType::Unknown */,
                                                       StyleFormat format /* = StyleFormat::QML */)
{
    std::string data;
    QFile file( QString::fromStdString( filePath) );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QByteArray byteArray = file.readAll();
        data = std::string( byteArray.constData(), byteArray.length() );
    }

    return HeadlessRender::Style::fromString( data, svgResolverCallback, layerGeometryType, layerType, format );
}

HeadlessRender::Style HeadlessRender::Style::fromDefaults( const QColor &color,
                                                           LayerGeometryType layerGeometryType /* = LayerGeometryType::Undefined */,
                                                           DataType layerType /* = DataType::Unknown */,
                                                           StyleFormat format /* = StyleFormat::QML */)
{
    Style style;
    style.init({
               color,
               layerGeometryType,
               layerType
           });
    return style;
}

QgsVectorLayerPtr Style::createTemporaryVectorLayerWithStyle( QString &errorMessage ) const
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
        if ( !importToLayer( qgsVectorLayer, styleData, errorMessage ))
            return nullptr;

        mCachedTemporaryLayer = qgsVectorLayer;
    }
    return std::dynamic_pointer_cast<QgsVectorLayer>( mCachedTemporaryLayer );
}

QgsRasterLayerPtr Style::createTemporaryRasterLayerWithStyle( QString &errorMessage) const
{
    if ( !mCachedTemporaryLayer )
    {
        HeadlessRender::QgsMapLayerPtr qgsRasterLayer = createTemporaryRasterLayer();

        QDomDocument styleData = mData;
        if ( !importToLayer( qgsRasterLayer, styleData, errorMessage ))
            return nullptr;

        mCachedTemporaryLayer = qgsRasterLayer;
    }
    return std::dynamic_pointer_cast<QgsRasterLayer>( mCachedTemporaryLayer );
}

void Style::init( const CreateParams &params )
{
    mData.setContent( params.data );

    if ( !validateGeometryType( params.layerGeometry ))
        throw StyleTypeMismatch( ErrorString::StyleMismatch );

    QString errorMessage;
    if ( !validateStyle( errorMessage ))
        throw StyleValidationError( errorMessage );

    if ( params.layerType != DataType::Unknown && type() != params.layerType )
        throw StyleTypeMismatch( ErrorString::LayerStyleMismatch );

    if (params.callback)
    {
        mData = resolveSvgPaths( params.callback );
        mCachedTemporaryLayer.reset();
    }
}

void Style::init(const DefaultStyleParams &params)
{
    mDefault = true;
    mDefaultStyleParams = params;
}

DataType Style::type() const
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

UsedAttributes Style::usedAttributes() const
{
    if ( !mUsedAttributesCached )
    {
        mUsedAttributesCache = readUsedAttributes();
        mUsedAttributesCached = true;
    }
    return mUsedAttributesCache;
}

ScaleRange Style::scaleRange() const
{
    if ( mScaleRange[0] == -1 )
    {
        if ( isDefaultStyle() )
            mScaleRange[0] = -2;
        else
        {
            QgsMapLayerPtr qgsMapLayer;
            QString errorMessage;

            if ( type() == DataType::Raster )
                qgsMapLayer = createTemporaryRasterLayerWithStyle( errorMessage );
            else
                qgsMapLayer = createTemporaryVectorLayerWithStyle( errorMessage );

            if ( !qgsMapLayer )
                throw QgisHeadlessError( errorMessage );

            if ( !qgsMapLayer->hasScaleBasedVisibility() )
                mScaleRange[0] = -2;
            else
            {
                mScaleRange[0] = qgsMapLayer->minimumScale();
                mScaleRange[1] = qgsMapLayer->maximumScale();
            }
        }
    }
    return mScaleRange;
}

bool Style::isDefaultStyle() const
{
    return mDefault;
}

bool Style::validateGeometryType( LayerGeometryType layerGeometryType ) const
{
    QDomElement geometryTypeElement = mData.firstChildElement( TAGS::QGIS ).firstChildElement( TAGS::LAYER_GEOMETRY_TYPE );
    if ( layerGeometryType == LayerGeometryType::Unknown || geometryTypeElement.isNull() )
        return true;

    QgsVectorLayer::LayerOptions layerOptions;
    layerOptions.fallbackWkbType = layerGeometryTypeToQgsWkbType( layerGeometryType );
    QgsVectorLayerPtr qgsVectorLayer = std::dynamic_pointer_cast<QgsVectorLayer>( createTemporaryVectorLayer( layerOptions ));

    QgsWkbTypes::GeometryType importLayerGeometryType = static_cast<QgsWkbTypes::GeometryType>( geometryTypeElement.text().toInt() );

    return ( qgsVectorLayer->geometryType() == importLayerGeometryType ? true : false );
}

bool Style::validateStyle( QString &errorMessage ) const
{
    if (type() == DataType::Vector)
        return createTemporaryVectorLayerWithStyle( errorMessage ) ? true : false;
    else
        return createTemporaryRasterLayerWithStyle( errorMessage ) ? true : false;
}

bool Style::importToLayer( HeadlessRender::QgsMapLayerPtr &layer, QString &errorMessage ) const
{

    if ( mCachedTemporaryLayer &&
         mCachedTemporaryLayer->type() == layer->type() &&
         !mCachedTemporaryLayer->styleManager()->styles().empty() )
    {
        const QString currentStyleName = mCachedTemporaryLayer->styleManager()->currentStyle();
        const QgsMapLayerStyle currentStyle = mCachedTemporaryLayer->styleManager()->style( currentStyleName );
        const QString styleName = "StyleName_" + QString::number(layer->styleManager()->styles().size());

        layer->styleManager()->addStyle( styleName, currentStyle );
        bool result = layer->styleManager()->setCurrentStyle( styleName );
        if ( !result )
        {
            errorMessage = ErrorString::AddStyleFailed;
        }
        return result;
    }
    else
    {
        return importToLayer( layer, mData, errorMessage );
    }
}

bool Style::importToLayer( QgsMapLayerPtr &layer, QDomDocument styleData, QString &errorMessage ) const
{
    return layer->importNamedStyle( styleData, errorMessage, static_cast<QgsMapLayer::StyleCategory>( Style::DefaultImportCategories ) );
}

UsedAttributes Style::readUsedAttributes() const
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
            QgsRuleBasedLabeling *ruleBasedLabeling = dynamic_cast<QgsRuleBasedLabeling *>( abstractVectorLayerLabeling );
            if ( ruleBasedLabeling->rootRule() )
                for ( const QgsRuleBasedLabeling::Rule *rule : ruleBasedLabeling->rootRule()->children() )
                    fields.unite( QgsExpression( rule->filterExpression() ).referencedColumns() );
        }

        for ( const QString &providerId : qgsVectorLayer->labeling()->subProviders() )
        {
            fields.unite( qgsVectorLayer->labeling()->settings( providerId ).referencedFields( renderContext ));
        }

        for ( const QString &field : fields )
            usedAttributes.insert( field.toStdString() );
    }

    for (const QString &attr : qgsVectorLayer->renderer()->usedAttributes( renderContext ))
        usedAttributes.insert( attr.toStdString() );

    return std::make_pair( true, usedAttributes );
}

bool Style::hasEnabledDiagrams( const QgsVectorLayerPtr &layer ) const
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

void Style::resolveSymbol( QgsSymbol *symbol, const HeadlessRender::SvgResolverCallback &svgResolverCallback ) const
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

QDomDocument Style::resolveSvgPaths( const HeadlessRender::SvgResolverCallback &svgResolverCallback) const
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

QSet<QString> Style::referencedFields( const HeadlessRender::QgsVectorLayerPtr &layer, const QgsRenderContext &context, const QString &providerId ) const
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

void Style::removeLayerGeometryTypeElement( QDomDocument &domDocument ) const
{
    QDomElement root = domDocument.firstChildElement( TAGS::QGIS );
    if ( !root.isNull() )
    {
        QDomElement child = root.firstChildElement( TAGS::LAYER_GEOMETRY_TYPE );
        if ( !child.isNull() )
            root.removeChild( child );
    }
}

QString Style::exportToString( const StyleFormat format ) const
{
    QgsMapLayerPtr qgsMapLayer;
    QString errorMessage;

    if ( isDefaultStyle() )
    {
        if ( mDefaultStyleParams.layerType == DataType::Raster )
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

        if ( mDefaultStyleParams.layerType != DataType::Raster )
        {
            QgsSymbol *qgsSymbol;
            QgsSymbolLayer *qgsSymbolLayer;

            QVariantMap props;
            props[QStringLiteral( "color" )] = mDefaultStyleParams.color.name( QColor::NameFormat::HexArgb );

            switch( mDefaultStyleParams.layerGeometryType )
            {
            case LayerGeometryType::Point:
            case LayerGeometryType::PointZ:
            case LayerGeometryType::MultiPoint:
            case LayerGeometryType::MultiPointZ:
                qgsSymbol = new QgsMarkerSymbol;
                qgsSymbolLayer = QgsSimpleMarkerSymbolLayer::create( props );
                break;
            case LayerGeometryType::LineString:
            case LayerGeometryType::LineStringZ:
            case LayerGeometryType::MultiLineString:
            case LayerGeometryType::MultiLineStringZ:
                qgsSymbol = new QgsLineSymbol;
                qgsSymbolLayer = QgsSimpleLineSymbolLayer::create( props );
                break;
            case LayerGeometryType::Polygon:
            case LayerGeometryType::PolygonZ:
            case LayerGeometryType::MultiPolygon:
            case LayerGeometryType::MultiPolygonZ:
                qgsSymbol = new QgsFillSymbol;
                qgsSymbolLayer = QgsSimpleFillSymbolLayer::create( props );
                break;
            case LayerGeometryType::Unknown:
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
        if ( type() == DataType::Raster )
            qgsMapLayer = createTemporaryRasterLayerWithStyle( errorMessage );
        else
            qgsMapLayer = createTemporaryVectorLayerWithStyle( errorMessage );

        if ( !qgsMapLayer )
            throw QgisHeadlessError( errorMessage );
    }

    QDomDocument exportedStyle;
    switch( format )
    {
        case StyleFormat::QML:
            qgsMapLayer->exportNamedStyle( exportedStyle, errorMessage,  QgsReadWriteContext(),  static_cast<QgsMapLayer::StyleCategory>( DefaultImportCategories ));
        break;
        case StyleFormat::SLD:
            qgsMapLayer->exportSldStyle( exportedStyle, errorMessage);
        break;
    }

    return exportedStyle.toString();
}

const QDomDocument &Style::data() const
{
    return mData;
}

QDomDocument &Style::data()
{
    return mData;
}

QColor Style::defaultStyleColor() const
{
    return mDefaultStyleParams.color;
}
