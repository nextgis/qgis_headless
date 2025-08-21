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

#include <qgscallout.h>
#include <qgsexpressioncontext.h>
#include <qgsfeaturerequest.h>
#include <qgsfillsymbol.h>
#include <qgsfillsymbollayer.h>
#include <qgslinesymbol.h>
#include <qgslinesymbollayer.h>
#include <qgsmaplayerstylemanager.h>
#include <qgsmarkersymbol.h>
#include <qgsmarkersymbollayer.h>
#include <qgspallabeling.h>
#include <qgspropertycollection.h>
#include <qgsrasterlayer.h>
#include <qgsrendercontext.h>
#include <qgsrenderer.h>
#include <qgsrulebasedlabeling.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsvectorlayerlabeling.h>

#include <QFile>
#include <QUrl>

#if VERSION_INT >= 33000
#include <qgsdiagramrenderer.h>
#endif

namespace HeadlessRender
{
  namespace TAGS
  {
    const QString QGIS = "qgis";
    const QString LAYER_GEOMETRY_TYPE = "layerGeometryType";
    const QString PIPE = "pipe";
    const QString PROVIDER = "provider";
    const QString RASTER_PROPERTIES = "rasterproperties";
    const QString RASTER_RENDERER = "rasterrenderer";
    const QString RESAMPLING = "resampling";
    const QString DIAGRAM_CATEGORY = "DiagramCategory";
    const QString ENABLED = "enabled";
    const QString STYLED_LAYER_DESCRIPTOR = "StyledLayerDescriptor";
    const QString NAMED_LAYER = "NamedLayer";
    const QString POLYGON_SYMBOLIZER = "PolygonSymbolizer";
    const QString LINE_SYMBOLIZER = "LineSymbolizer";
  } //namespace TAGS

  namespace SymbolLayerType
  {
    const QString SvgMarker = "SvgMarker";
    const QString SVGFill = "SVGFill";
  } //namespace SymbolLayerType

  namespace LabelingType
  {
    const QString RuleBased = "rule-based";
  }

  namespace ErrorString
  {
    const QString StyleMismatch = "Style type mismatch";
    const QString LayerStyleMismatch = "Layer type and style type do not match";
    const QString AddStyleFailed = "AddStyle failed";
  } //namespace ErrorString
} //namespace HeadlessRender

using namespace HeadlessRender;

const StyleCategory Style::DefaultImportCategories = QgsMapLayer::Symbology
                                                     | QgsMapLayer::Symbology3D
                                                     | QgsMapLayer::Labeling | QgsMapLayer::Rendering
                                                     | QgsMapLayer::CustomProperties
                                                     | QgsMapLayer::Diagrams;

namespace
{
  bool isSvgPathResolvable( const QString &path )
  {
    const QUrl url( path );
    return url.isLocalFile() || url.isRelative();
  }

  QString resolveSvgPath( const QString &path, const SvgResolverCallback &svgResolverCallback )
  {
    if ( !svgResolverCallback || !isSvgPathResolvable( path ) )
    {
      return path;
    }
    return QString::fromStdString( svgResolverCallback( path.toStdString() ) );
  }

  void resolveSymbol( QgsSymbol *symbol, const SvgResolverCallback &svgResolverCallback )
  {
    for ( QgsSymbolLayer *symbolLayer : symbol->symbolLayers() )
    {
      if ( symbolLayer->layerType() == SymbolLayerType::SvgMarker )
      {
        auto svgMarkerSymbolLayer = dynamic_cast<QgsSvgMarkerSymbolLayer *>( symbolLayer );
        const QColor fillColor = svgMarkerSymbolLayer->fillColor();
        const QColor strokeColor = svgMarkerSymbolLayer->strokeColor();
        const double strokeWidth = svgMarkerSymbolLayer->strokeWidth();

        svgMarkerSymbolLayer->setPath(
          resolveSvgPath( svgMarkerSymbolLayer->path(), svgResolverCallback )
        );

        svgMarkerSymbolLayer->setFillColor( fillColor );
        svgMarkerSymbolLayer->setStrokeColor( strokeColor );
        svgMarkerSymbolLayer->setStrokeWidth( strokeWidth );
      }
      else if ( symbolLayer->layerType() == SymbolLayerType::SVGFill )
      {
        auto svgFillSymbolLayer = dynamic_cast<QgsSVGFillSymbolLayer *>( symbolLayer );

        auto path = svgFillSymbolLayer->svgFilePath();
        svgFillSymbolLayer->setSvgFilePath( resolveSvgPath( path, svgResolverCallback ) );
      }

      if ( symbolLayer->subSymbol() )
      {
        resolveSymbol( symbolLayer->subSymbol(), svgResolverCallback );
      }
    }
  }

  void resolveLabelingSvgPaths(
    QgsAbstractVectorLayerLabeling *labeling, const SvgResolverCallback &svgResolverCallback
  )
  {
    auto settings = std::make_unique<QgsPalLayerSettings>( labeling->settings() );
    auto format = settings->format();
    auto &&backgound = format.background();
    if ( backgound.type() == QgsTextBackgroundSettings::ShapeMarkerSymbol )
    {
      resolveSymbol( backgound.markerSymbol(), svgResolverCallback );
    }
    else if ( backgound.type() == QgsTextBackgroundSettings::ShapeSVG )
    {
      backgound.setSvgFile( resolveSvgPath( backgound.svgFile(), svgResolverCallback ) );
    }
    settings->setFormat( format );
    labeling->setSettings( settings.release() );
  }
} //namespace

Style Style::fromString(
  const std::string &data, const SvgResolverCallback &svgResolverCallback /* = nullptr */,
  LayerGeometryType layerGeometryType /* = LayerGeometryType::Undefined */,
  DataType layerType /* = DataType::Unknown */, StyleFormat format /* = StyleFormat::QML */
)
{
  Style style;
  switch ( format )
  {
    case StyleFormat::QML:
      style.init(
        { QString::fromStdString( data ), svgResolverCallback, layerGeometryType, layerType }
      );
      break;
    case StyleFormat::SLD:

      if ( layerType == DataType::Raster )
      {
        throw StyleTypeMismatch( "Raster layers do not support importing SLD styles" );
      }

      QDomDocument styleDom;
      styleDom.setContent( QString::fromStdString( data ), true );

      const QDomElement myRoot = styleDom.firstChildElement( TAGS::STYLED_LAYER_DESCRIPTOR );
      const QDomElement namedLayerElem = myRoot.firstChildElement( TAGS::NAMED_LAYER );

      QgsVectorLayer::LayerOptions layerOptions;
      if ( namedLayerElem.elementsByTagName( TAGS::POLYGON_SYMBOLIZER ).size() != 0 )
        layerOptions.fallbackWkbType = Qgis::WkbType::Polygon;
      else if ( namedLayerElem.elementsByTagName( TAGS::LINE_SYMBOLIZER ).size() != 0 )
        layerOptions.fallbackWkbType = Qgis::WkbType::LineString;
      else
        layerOptions.fallbackWkbType = Qgis::WkbType::Point;

      QgsMapLayerPtr layer = createTemporaryLayerByType( layerType, layerOptions );
      QString errorMessage;
      if ( !layer->readSld( namedLayerElem, errorMessage ) )
        throw StyleValidationError( "Cannot import SLD style, error: " + errorMessage );

      layer->exportNamedStyle( styleDom, errorMessage, {}, static_cast<QgsMapLayer::StyleCategory>( DefaultImportCategories ) );
      style.init( { styleDom.toString(), svgResolverCallback, layerGeometryType, layerType } );

      break;
  }

  return style;
}

Style Style::fromFile(
  const std::string &filePath, const SvgResolverCallback &svgResolverCallback /* = nullptr */,
  LayerGeometryType layerGeometryType /* = LayerGeometryType::Unknown */,
  DataType layerType /* = DataType::Unknown */, StyleFormat format /* = StyleFormat::QML */
)
{
  std::string data;
  QFile file( QString::fromStdString( filePath ) );
  if ( file.open( QIODevice::ReadOnly ) )
  {
    QByteArray byteArray = file.readAll();
    data = std::string( byteArray.constData(), byteArray.length() );
  }

  return Style::fromString( data, svgResolverCallback, layerGeometryType, layerType, format );
}

Style Style::fromDefaults(
  const QColor &color, LayerGeometryType layerGeometryType /* = LayerGeometryType::Undefined */,
  DataType layerType /* = DataType::Unknown */
)
{
  Style style;
  style.init( { color, layerGeometryType, layerType } );
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
      switch ( static_cast<Qgis::GeometryType>(
        myRoot.firstChildElement( TAGS::LAYER_GEOMETRY_TYPE ).text().toInt()
      ) )
      {
        case Qgis::GeometryType::Point:
          layerOptions.fallbackWkbType = Qgis::WkbType::Point;
          break;
        case Qgis::GeometryType::Line:
          layerOptions.fallbackWkbType = Qgis::WkbType::LineString;
          break;
        case Qgis::GeometryType::Polygon:
          layerOptions.fallbackWkbType = Qgis::WkbType::Polygon;
          break;
        case Qgis::GeometryType::Unknown:
          layerOptions.fallbackWkbType = Qgis::WkbType::Unknown;
          break;
        case Qgis::GeometryType::Null:
          layerOptions.fallbackWkbType = Qgis::WkbType::NoGeometry;
          break;
      }
    }
    else
    {
      layerOptions.fallbackWkbType = Qgis::WkbType::Point;
    }

    QgsMapLayerPtr qgsVectorLayer = createTemporaryVectorLayer( layerOptions );
    if ( !importToLayer( qgsVectorLayer, styleData, errorMessage ) )
      return nullptr;

    mCachedTemporaryLayer = qgsVectorLayer;
  }
  return std::dynamic_pointer_cast<QgsVectorLayer>( mCachedTemporaryLayer );
}

QgsRasterLayerPtr Style::createTemporaryRasterLayerWithStyle( QString &errorMessage ) const
{
  if ( !mCachedTemporaryLayer )
  {
    QgsMapLayerPtr qgsRasterLayer = createTemporaryRasterLayer();

    QDomDocument styleData = mData;
    if ( !importToLayer( qgsRasterLayer, styleData, errorMessage ) )
      return nullptr;

    mCachedTemporaryLayer = qgsRasterLayer;
  }
  return std::dynamic_pointer_cast<QgsRasterLayer>( mCachedTemporaryLayer );
}

QgsMapLayerPtr Style::createTemporaryLayerWithStyleByType( const DataType type, QString &errorMessage ) const
{
  if ( type == DataType::Raster )
    return createTemporaryRasterLayerWithStyle( errorMessage );
  else
    return createTemporaryVectorLayerWithStyle( errorMessage );
}

void Style::init( const CreateParams &params )
{
  mData.setContent( params.data );

  if ( !validateGeometryType( params.layerGeometry ) )
    throw StyleTypeMismatch( ErrorString::StyleMismatch );

  QString errorMessage;
  if ( !validateStyle( errorMessage ) )
    throw StyleValidationError( errorMessage );

  if ( params.layerType != DataType::Unknown && type() != params.layerType )
    throw StyleTypeMismatch( ErrorString::LayerStyleMismatch );

  if ( params.callback || type() == DataType::Vector )
  {
    mData = resolveSvgPaths( params.callback );
    mCachedTemporaryLayer.reset();
  }
}

void Style::init( const DefaultStyleParams &params )
{
  mDefault = true;
  mDefaultStyleParams = params;
}

DataType Style::type() const
{
  if ( mType == DataType::Unknown && !mData.isNull() )
  {
    bool isRaster = false;
    QDomElement root = mData.firstChildElement( TAGS::QGIS );

    QDomNode pipeNode = root.firstChildElement( TAGS::PIPE );
    if ( pipeNode.isNull() )
    {
      // old project
      pipeNode = root;
    }
    else
    {
      QDomNode providerNode = pipeNode.firstChildElement( TAGS::PROVIDER );
      if ( !providerNode.isNull() )
      {
        QDomNode resamplingNode = providerNode.firstChildElement( TAGS::RESAMPLING );
        isRaster = !resamplingNode.isNull();
      }
    }

    if ( !isRaster )
    {
      QDomElement rendererElement;
      //rasterlayerproperties element there -> old format (1.8 and early 1.9)
      if ( !root.firstChildElement( TAGS::RASTER_PROPERTIES ).isNull() )
        rendererElement = root.firstChildElement( TAGS::RASTER_RENDERER );
      else
        rendererElement = pipeNode.firstChildElement( TAGS::RASTER_RENDERER );

      isRaster = !rendererElement.isNull();
    }

    mType = isRaster ? DataType::Raster : DataType::Vector;
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
      QString errorMessage;
      QgsMapLayerPtr qgsMapLayer = createTemporaryLayerWithStyleByType( type(), errorMessage );
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
  QDomElement geometryTypeElement = mData.firstChildElement( TAGS::QGIS )
                                      .firstChildElement( TAGS::LAYER_GEOMETRY_TYPE );
  if ( layerGeometryType == LayerGeometryType::Unknown || geometryTypeElement.isNull() )
    return true;

  QgsVectorLayer::LayerOptions layerOptions;
  layerOptions.fallbackWkbType = layerGeometryTypeToQgsWkbType( layerGeometryType );
  QgsVectorLayerPtr qgsVectorLayer = std::dynamic_pointer_cast<QgsVectorLayer>(
    createTemporaryVectorLayer( layerOptions )
  );
  Qgis::GeometryType importLayerGeometryType = static_cast<Qgis::GeometryType>(
    geometryTypeElement.text().toInt()
  );
  return ( qgsVectorLayer->geometryType() == importLayerGeometryType ? true : false );
}

bool Style::validateStyle( QString &errorMessage ) const
{
  return createTemporaryLayerWithStyleByType( type(), errorMessage ) ? true : false;
}

bool Style::importToLayer( QgsMapLayerPtr &layer, QString &errorMessage ) const
{
  if ( mCachedTemporaryLayer && mCachedTemporaryLayer->type() == layer->type()
       && !mCachedTemporaryLayer->styleManager()->styles().empty() )
  {
    const QString currentStyleName = mCachedTemporaryLayer->styleManager()->currentStyle();
    const QgsMapLayerStyle currentStyle = mCachedTemporaryLayer->styleManager()->style(
      currentStyleName
    );
    const QString styleName = "StyleName_"
                              + QString::number( layer->styleManager()->styles().size() );

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
  return layer
    ->importNamedStyle( styleData, errorMessage, static_cast<QgsMapLayer::StyleCategory>( Style::DefaultImportCategories ) );
}

UsedAttributes Style::readUsedAttributes() const
{
  std::set<std::string> usedAttributes;

  if ( isDefaultStyle() )
  {
    return std::make_pair( true, usedAttributes );
  }

  QString errorMessage;
  QgsVectorLayerPtr qgsVectorLayer = createTemporaryVectorLayerWithStyle( errorMessage );
  if ( !qgsVectorLayer )
  {
    throw QgisHeadlessError( errorMessage );
  }

  if ( qgsVectorLayer->diagramsEnabled() )
  {
#if VERSION_INT >= 33000
    const QgsDiagramRenderer *diagramRenderer = qgsVectorLayer->diagramRenderer();
    if ( diagramRenderer )
    {
      for ( auto &&attribute : diagramRenderer->referencedFields() )
      {
        usedAttributes.insert( attribute.toStdString() );
      }
    }

    const QgsDiagramLayerSettings *diagramSettings = qgsVectorLayer->diagramLayerSettings();
    if ( diagramSettings )
    {
      for ( auto &&attribute : diagramSettings->referencedFields() )
      {
        usedAttributes.insert( attribute.toStdString() );
      }

      const QgsPropertyCollection &dataProperties = diagramSettings->dataDefinedProperties();
      for ( const QString &field : dataProperties.referencedFields( QgsExpressionContext(), true ) )
      {
        usedAttributes.insert( field.toStdString() );
      }
    }
#else
    return std::make_pair( false, usedAttributes );
#endif
  }

  QgsRenderContext renderContext;

  QgsAbstractVectorLayerLabeling *labeling = qgsVectorLayer->labeling();
  if ( labeling )
  {
    QSet<QString> fields;

    if ( labeling->type() == LabelingType::RuleBased )
    {
      QgsRuleBasedLabeling *ruleBasedLabeling = dynamic_cast<QgsRuleBasedLabeling *>( labeling );
      if ( ruleBasedLabeling && ruleBasedLabeling->rootRule() )
      {
        for ( const QgsRuleBasedLabeling::Rule *rule : ruleBasedLabeling->rootRule()->children() )
        {
          fields.unite( QgsExpression( rule->filterExpression() ).referencedColumns() );
        }
      }
    }

    for ( const QString &providerId : labeling->subProviders() )
    {
      fields.unite( labeling->settings( providerId ).referencedFields( renderContext ) );
    }

    for ( const QString &field : fields )
    {
      usedAttributes.insert( field.toStdString() );
    }
  }

  auto &&renderer = qgsVectorLayer->renderer();
  for ( const QString &attribute : renderer->usedAttributes( renderContext ) )
  {
    if ( attribute == QgsFeatureRequest::ALL_ATTRIBUTES )
    {
      return std::make_pair( false, usedAttributes );
    }
    usedAttributes.insert( attribute.toStdString() );
  }

  if ( renderer->orderByEnabled() )
  {
    for ( auto &&attribute : renderer->orderBy().usedAttributes() )
    {
      usedAttributes.insert( attribute.toStdString() );
    }
  }

  return std::make_pair( true, std::move( usedAttributes ) );
}

QDomDocument Style::resolveSvgPaths( const SvgResolverCallback &svgResolverCallback ) const
{
  QDomDocument exportedStyle;
  QString errorMessage;

  QgsVectorLayerPtr qgsVectorLayer = createTemporaryVectorLayerWithStyle( errorMessage );
  if ( !qgsVectorLayer )
    throw QgisHeadlessError( errorMessage );

  QgsRenderContext renderContext;
  for ( QgsSymbol *symbol : qgsVectorLayer->renderer()->symbols( renderContext ) )
  {
    resolveSymbol( symbol, svgResolverCallback );
  }

  if ( auto &&labeling = qgsVectorLayer->labeling() )
  {
    resolveLabelingSvgPaths( labeling, svgResolverCallback );
  }

  qgsVectorLayer->exportNamedStyle( exportedStyle, errorMessage );
  if ( !errorMessage.isEmpty() )
    throw QgisHeadlessError( errorMessage );

  QDomElement myRoot = mData.firstChildElement( TAGS::QGIS );
  if ( myRoot.isNull() || myRoot.firstChildElement( TAGS::LAYER_GEOMETRY_TYPE ).text().isEmpty() )
    removeLayerGeometryTypeElement( exportedStyle );

  return exportedStyle;
}

QSet<QString> Style::referencedFields(
  const QgsVectorLayerPtr &layer, const QgsRenderContext &context, const QString &providerId
) const
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

  referenced.unite( settings.dataDefinedProperties().referencedFields( context.expressionContext() )
  );

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
    if ( mDefaultStyleParams.layerType != DataType::Raster )
    {
      QgsVectorLayer::LayerOptions layerOptions;
      layerOptions.fallbackWkbType = layerGeometryTypeToQgsWkbType(
        mDefaultStyleParams.layerGeometryType
      );
      qgsMapLayer = createTemporaryVectorLayer( layerOptions );
      if ( !qgsMapLayer )
        throw QgisHeadlessError( errorMessage );

      std::unique_ptr<QgsSymbol> qgsSymbol;
      std::unique_ptr<QgsSymbolLayer> qgsSymbolLayer;

      QVariantMap props;
      props[QStringLiteral( "color" )] = mDefaultStyleParams.color.name( QColor::NameFormat::HexArgb );

      switch ( mDefaultStyleParams.layerGeometryType )
      {
        case LayerGeometryType::Point:
        case LayerGeometryType::PointZ:
        case LayerGeometryType::MultiPoint:
        case LayerGeometryType::MultiPointZ:
          qgsSymbol = std::make_unique<QgsMarkerSymbol>();
          qgsSymbolLayer.reset( QgsSimpleMarkerSymbolLayer::create( props ) );
          break;
        case LayerGeometryType::LineString:
        case LayerGeometryType::LineStringZ:
        case LayerGeometryType::MultiLineString:
        case LayerGeometryType::MultiLineStringZ:
          qgsSymbol = std::make_unique<QgsLineSymbol>();
          qgsSymbolLayer.reset( QgsSimpleLineSymbolLayer::create( props ) );
          break;
        case LayerGeometryType::Polygon:
        case LayerGeometryType::PolygonZ:
        case LayerGeometryType::MultiPolygon:
        case LayerGeometryType::MultiPolygonZ:
          qgsSymbol = std::make_unique<QgsFillSymbol>();
          qgsSymbolLayer.reset( QgsSimpleFillSymbolLayer::create( props ) );
          break;
        case LayerGeometryType::Unknown:
          // Both qgsSymbol and qgsSymbolLayer are nullptr
          break;
      }

      if ( qgsSymbol && qgsSymbolLayer )
      {
        qgsSymbol->changeSymbolLayer( 0, qgsSymbolLayer.release() );
        QgsSingleSymbolRenderer *qgsSingleSymbolRenderer = new QgsSingleSymbolRenderer(
          qgsSymbol.release()
        );
        static_cast<QgsVectorLayer *>( qgsMapLayer.get() )->setRenderer( qgsSingleSymbolRenderer );
      }
    }
    else
    {
      qgsMapLayer = createTemporaryRasterLayer();
      if ( !qgsMapLayer )
        throw QgisHeadlessError( errorMessage );
    }
  }
  else
  {
    qgsMapLayer = createTemporaryLayerWithStyleByType( type(), errorMessage );
    if ( !qgsMapLayer )
      throw QgisHeadlessError( errorMessage );
  }

  QDomDocument exportedStyle;
  switch ( format )
  {
    case StyleFormat::QML:
      qgsMapLayer
        ->exportNamedStyle( exportedStyle, errorMessage, QgsReadWriteContext(), static_cast<QgsMapLayer::StyleCategory>( DefaultImportCategories ) );
      break;
    case StyleFormat::SLD:
      qgsMapLayer->exportSldStyle( exportedStyle, errorMessage );
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
