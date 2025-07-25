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
#include <qgslegendrenderer.h>
#include <qgslegendsettings.h>
#include <qgslayertreemodel.h>
#include <qgslayertree.h>
#include <qgsrendercontext.h>
#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsrasterrenderer.h>
#include <qgslayoutexporter.h>
#include <qgsmaprenderercustompainterjob.h>
#include <qgsexpressioncontextutils.h>
#include <qgsrenderer.h>
#include <qgsmultibandcolorrenderer.h>
#include <qgspalettedrasterrenderer.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgssinglebandpseudocolorrenderer.h>
#include <qgsrastershader.h>
#include <qgscolorrampshader.h>

#include "exceptions.h"

#include <QApplication>
#include <QSizeF>
#include <QPrinter>
#include <QJsonArray>
#include <cstdlib>

namespace
{
  QApplication *app = nullptr;
  HeadlessRender::LogLevel appLogLevel = HeadlessRender::LogLevel::Debug;

  const auto SymbolRenderingNotAdjustableError = QStringLiteral( "Symbol rendering is not adjustable" );
  const auto InvalidSymbolIndexError = QStringLiteral( "Invalid symbol index" );
  const auto InvalidLayerIndexError = QStringLiteral( "Invalid layer index" );

  namespace KEYS
  {
    const auto TYPE = QStringLiteral( "type" );
    const auto SYMBOLS = QStringLiteral( "symbols" );
    const auto NODES = QStringLiteral( "nodes" );
    const auto ICON = QStringLiteral( "icon" );
    const auto TITLE = QStringLiteral( "title" );
  } //namespace KEYS

  namespace RendererType
  {
    const auto MULTIBANDCOLOR = QStringLiteral( "multibandcolor" );
    const auto PALETTED = QStringLiteral( "paletted" );
    const auto SINGLEBANDGRAY = QStringLiteral( "singlebandgray" );
    const auto SINGLEBANDPSEUDOCOLOR = QStringLiteral( "singlebandpseudocolor" );
  } //namespace RendererType

  void messageHandler( QtMsgType msgType, const QMessageLogContext &, const QString &msg )
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

  /**
   * Creates a copy of current global expression context, including mapSettings' scope
   */
  QgsExpressionContext createExpressionContext( HeadlessRender::QgsMapSettingsPtr mapSettings )
  {
    QgsExpressionContext expressionContext;
    expressionContext << QgsExpressionContextUtils::globalScope()
                      << QgsExpressionContextUtils::atlasScope( nullptr )
                      << QgsExpressionContextUtils::mapSettingsScope( *mapSettings )
                      << new QgsExpressionContextScope;

    return expressionContext;
  }

  QColor interpolateColors( const QColor &color1, const QColor &color2, qreal ratio )
  {
    qreal inverseRatio = 1.0 - ratio;
    return QColor(
      color1.red() * inverseRatio + color2.red() * ratio,
      color1.green() * inverseRatio + color2.green() * ratio,
      color1.blue() * inverseRatio + color2.blue() * ratio,
      color1.alpha() * inverseRatio + color2.alpha() * ratio
    );
  }
} // namespace

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
  for ( const std::string &path : paths )
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

const char *HeadlessRender::getVersion()
{
  return QGIS_HEADLESS_LIB_VERSION_STRING;
}

const char *HeadlessRender::getQGISVersion()
{
  return VERSION;
}

int HeadlessRender::getQGISVersionInt()
{
  return _QGIS_VERSION_INT;
}

HeadlessRender::MapRequest::MapRequest()
  : mSettings( std::make_shared<QgsMapSettings>() )
  , mQgsLayerTree( std::make_shared<QgsLayerTree>() )
{
  mSettings->setBackgroundColor( Qt::transparent );
  mSettings->setFlag( Qgis::MapSettingsFlag::RenderBlocking );
}

void HeadlessRender::MapRequest::setDpi( int dpi )
{
  mSettings->setOutputDpi( dpi );
}

void HeadlessRender::MapRequest::setCrs( const HeadlessRender::CRS &crs )
{
  mSettings->setDestinationCrs( *crs.qgsCoordinateReferenceSystem() );
}

HeadlessRender::LayerIndex HeadlessRender::MapRequest::
  addLayer( HeadlessRender::Layer &layer, Style &style, const std::string &label /* = "" */ )
{
  QgsMapLayerPtr qgsMapLayer = layer.qgsMapLayer();
  if ( !qgsMapLayer )
    throw QgisHeadlessError( QStringLiteral( "Layer is null" ) );

  if ( style.isDefaultStyle() )
  {
    layer.setRendererSymbolColor( style.defaultStyleColor() );
  }
  else
  {
    QString readStyleError;
    if ( !layer.addStyle( style, readStyleError ) )
      throw QgisHeadlessError( QStringLiteral( "Cannot add style, error message: " ) + readStyleError );
  }

  qgsMapLayer->setName( QString::fromStdString( label ) );

  mLayers.push_back( qgsMapLayer );

  QList<QgsMapLayer *> qgsMapLayers;
  for ( const QgsMapLayerPtr &layer : mLayers )
    qgsMapLayers.push_back( layer.get() );
  mSettings->setLayers( qgsMapLayers );

  mQgsLayerTree->addLayer( qgsMapLayer.get() );

  const auto addedLayerIndex = qgsMapLayers.size() - 1;

  if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( qgsMapLayer.get() ) )
  {
    if ( vlayer->renderer() )
    {
      SymbolIndexVector renderSymbols;
      int idx = 0;
      for ( const auto &symbolItem : vlayer->renderer()->legendSymbolItems() )
      {
        if ( vlayer->renderer()->legendSymbolItemChecked( symbolItem.ruleKey() ) )
          renderSymbols.push_back( idx );
        ++idx;
      }
      mDefaultRenderSymbols[addedLayerIndex] = renderSymbols;
    }
  }

  return addedLayerIndex;
}

void HeadlessRender::MapRequest::addProject( const Project &project )
{
  for ( const HeadlessRender::Layer &layer : project.layers() )
    mLayers.push_back( layer.qgsMapLayer() );

  QList<QgsMapLayer *> qgsMapLayers;
  for ( const QgsMapLayerPtr &layer : mLayers )
    qgsMapLayers.push_back( layer.get() );
  mSettings->setLayers( qgsMapLayers );
}

HeadlessRender::ImagePtr HeadlessRender::MapRequest::
  renderImage( const Extent &extent, const Size &size, const RenderSymbols &symbols /* = {} */ )
{
  const auto minx = std::get<0>( extent );
  const auto miny = std::get<1>( extent );
  const auto maxx = std::get<2>( extent );
  const auto maxy = std::get<3>( extent );

  const auto width = std::get<0>( size );
  const auto height = std::get<1>( size );

  QImage img( width, height, QImage::Format_ARGB32_Premultiplied );
  img.fill( Qt::transparent );

  QPainter painter( &img );

  prepareForRendering( { width, height }, QgsRectangle( minx, miny, maxx, maxy ) );

  applyRenderSymbols( symbols.empty() ? mDefaultRenderSymbols : symbols );

  QgsMapRendererCustomPainterJob job( *mSettings, &painter );
  job.renderSynchronously();

  return std::make_shared<HeadlessRender::Image>( img );
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
    img = QImage( width, height, QImage::Format_ARGB32_Premultiplied );

  img.fill( Qt::transparent );

  QPainter painter( &img );
  painter.setRenderHint( QPainter::Antialiasing, true );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );

  context.painter()->scale( dpmm, dpmm );

  legendRenderer.drawLegend( context );
  painter.end();

  return std::make_shared<HeadlessRender::Image>( img );
}

void HeadlessRender::MapRequest::exportPdf(
  const std::string &filepath, const Extent &extent, const HeadlessRender::Size &size
)
{
  double minx = std::get<0>( extent );
  double miny = std::get<1>( extent );
  double maxx = std::get<2>( extent );
  double maxy = std::get<3>( extent );

  int width = std::get<0>( size );
  int height = std::get<1>( size );

  prepareForRendering( { width, height }, QgsRectangle( minx, miny, maxx, maxy ) );

  QPrinter printer;
  printer.setOutputFileName( QString::fromStdString( filepath ) );
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

static void processLegendGroup(
  const QList<QgsLayerTreeNode *> &group, std::vector<HeadlessRender::LegendSymbol> &result,
  QgsLayerTreeModel &model, const QgsLegendSettings &settings,
  QgsLayerTreeModelLegendNode::ItemContext &context, QImage &image,
  HeadlessRender::LegendSymbol::Index index = 0, QgsRasterRenderer *rasterRenderer = nullptr,
  QgsFeatureRenderer *featureRenderer = nullptr,
  const int count = HeadlessRender::DefaultRasterRenderSymbolCount
)
{
  auto createLegendSymbol = [&](
                              QgsLayerTreeModelLegendNode *node,
                              const QList<QgsLayerTreeModelLegendNode *> &nodes,
                              const QString &title, const int rasterBand, const bool hasTitle = true
                            ) {
    auto symbolRender = HeadlessRender::SymbolRender::Uncheckable;
    if ( rasterRenderer )
    {
      const auto icon = node->data( Qt::DecorationRole ).value<QIcon>();
      if ( icon.isNull() )
        return;
    }
    else if ( featureRenderer->legendSymbolItemsCheckable() )
      symbolRender = node->data( Qt::CheckStateRole ).toBool()
                       ? HeadlessRender::SymbolRender::Checked
                       : HeadlessRender::SymbolRender::Unchecked;

    image.fill( Qt::transparent );
    node->draw( settings, &context );

    auto legendSymbol = HeadlessRender::LegendSymbol::
      create( std::make_shared<HeadlessRender::Image>( image ), title, symbolRender, index++, rasterBand, hasTitle );
    if ( nodes.size() == 1 && result.empty() )
      legendSymbol.setHasCategory( false );
    result.push_back( legendSymbol );
  };

  auto createLegendSymbolWithImage =
    [&]( QgsLayerTreeModelLegendNode *node, const QList<QgsLayerTreeModelLegendNode *> &nodes, const QImage &image, const QString &title, const int rasterBand ) {
      auto legendSymbol = HeadlessRender::LegendSymbol::
        create( std::make_shared<HeadlessRender::Image>( image ), title, HeadlessRender::SymbolRender::Uncheckable, index++, rasterBand );
      if ( nodes.size() == 1 && result.empty() )
        legendSymbol.setHasCategory( false );
      result.push_back( legendSymbol );
    };

  for ( const auto &it : group )
  {
    if ( QgsLayerTree::isLayer( it ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( it );
      const auto nodes = model.layerLegendNodes( nodeLayer );
      for ( const auto &node : nodes )
      {
        auto title = node->data( Qt::DisplayRole ).toString();
        int rasterBand = 0;
        if ( rasterRenderer )
        {
          if ( rasterRenderer->type() == RendererType::MULTIBANDCOLOR )
          {
            title = QString();
            if ( auto *r = dynamic_cast<QgsMultiBandColorRenderer *>( rasterRenderer ) )
            {
              switch ( index )
              {
                case 0:
                  rasterBand = r->redBand();
                  break;
                case 1:
                  rasterBand = r->greenBand();
                  break;
                case 2:
                  rasterBand = r->blueBand();
                  break;
              }
            }
            createLegendSymbol( node, nodes, title, rasterBand, false );

            if ( index > 3 )
              break;
          }
          else if ( rasterRenderer->type() == RendererType::PALETTED )
          {
            if ( auto *r = dynamic_cast<QgsPalettedRasterRenderer *>( rasterRenderer ) )
              rasterBand = r->band();

            createLegendSymbol( node, nodes, title, rasterBand );
          }
          else if ( rasterRenderer->type() == RendererType::SINGLEBANDGRAY )
          {
            if ( auto *r = dynamic_cast<QgsSingleBandGrayRenderer *>( rasterRenderer ) )
            {
              rasterBand = r->grayBand();

              const auto contrastEnhancement = r->contrastEnhancement();
              const auto min = contrastEnhancement->minimumValue();
              const auto max = contrastEnhancement->maximumValue();
              const auto step = ( max - min ) / ( count - 1.0 );
              const auto interpStep = 1.0 / ( count - 1.0 );
              const auto blackToWhite = r->gradient() == QgsSingleBandGrayRenderer::BlackToWhite;

              for ( auto i = 0; i < count; ++i )
              {
                image.fill(
                  interpolateColors( blackToWhite ? Qt::black : Qt::white, blackToWhite ? Qt::white : Qt::black, i * interpStep )
                );
                title = QString::number( ceil( blackToWhite ? i * step : max - i * step ) );
                createLegendSymbolWithImage( node, nodes, image, title, rasterBand );
              }
              break;
            }
          }
          else if ( rasterRenderer->type() == RendererType::SINGLEBANDPSEUDOCOLOR )
          {
            if ( auto *r = dynamic_cast<QgsSingleBandPseudoColorRenderer *>( rasterRenderer ) )
            {
              if ( auto *rampShader = dynamic_cast<QgsColorRampShader *>(
                     r->shader()->rasterShaderFunction()
                   ) )
              {
#if _QGIS_VERSION_INT < 33800
                if ( rampShader->colorRampType() == QgsColorRampShader::Type::Interpolated )
#else
                if ( rampShader->colorRampType() == Qgis::ShaderInterpolationMethod::Linear )
#endif
                {
                  auto addColorRampItem = [&]( const auto &item ) {
                    image.fill( item.color );
                    title = QString::number( item.value );
                    createLegendSymbolWithImage( node, nodes, image, title, rasterBand );
                  };

                  rasterBand = r->band();
                  const auto &colorRampItemList = rampShader->colorRampItemList();

                  addColorRampItem( colorRampItemList.first() );

                  double step = static_cast<double>( colorRampItemList.size() - 1 ) / ( count - 1 );
                  for ( auto i = 1; i < count - 1; ++i )
                  {
                    const double position = i * step;
                    const auto index = static_cast<int>( position );
                    addColorRampItem( colorRampItemList[index] );
                  }

                  addColorRampItem( colorRampItemList.last() );
                  break;
                }
                else
                  createLegendSymbol( node, nodes, title, rasterBand );
              }
            }
          }
          else
            createLegendSymbol( node, nodes, title, rasterBand );
        }
        else
          createLegendSymbol( node, nodes, title, rasterBand );
      }
    }
    else
    {
      const auto group = QgsLayerTree::toGroup( it );
      processLegendGroup( group->children(), result, model, settings, context, image, index, rasterRenderer, featureRenderer );
    }
  }
}

std::vector<HeadlessRender::LegendSymbol> HeadlessRender::MapRequest::
  legendSymbols( const LayerIndex index, const HeadlessRender::Size &size /* = Size() */, const int count /* = InvalidValue */ )
{
  if ( mLayers.size() <= index )
    throw QgisHeadlessError( InvalidLayerIndexError );

  int width = std::get<0>( size );
  int height = std::get<1>( size );

  QgsMapLayerPtr layer = mLayers.at( index );
  QgsRasterRenderer *rasterRenderer = nullptr;
  QgsFeatureRenderer *featureRenderer = nullptr;
  if ( auto *rasterLayer = qobject_cast<QgsRasterLayer *>( layer.get() ) )
    rasterRenderer = rasterLayer->renderer();
  else
    featureRenderer = qobject_cast<QgsVectorLayer *>( layer.get() )->renderer();

  QgsLayerTree qgsLayerTree;
  qgsLayerTree.addLayer( layer.get() );

  QgsLayerTreeModel legendModel( &qgsLayerTree );

  QImage image( width, height, QImage::Format_ARGB32_Premultiplied );

  QPainter p( &image );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &p );
  context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );

  int dpi = mSettings->outputDpi();
  qreal dpmm = dpi / 25.4;
  context.painter()->scale( dpmm, dpmm );

  qreal canvasFrac = 0.8;

  QgsLayerTreeModelLegendNode::ItemContext ctx;
  ctx.context = &context;
  ctx.painter = context.painter();
  ctx.columnLeft = ( 1 - canvasFrac ) / 2 * width / dpmm;
  ctx.top = ( 1 - canvasFrac ) / 2 * height / dpmm;

  QgsLegendSettings legendSettings;
  legendSettings.setSymbolSize( QSizeF( canvasFrac * width / dpmm, canvasFrac * height / dpmm ) );
  legendSettings.setMaximumSymbolSize( canvasFrac * height / dpmm );

  std::vector<HeadlessRender::LegendSymbol> legendSymbols;
  processLegendGroup( legendModel.rootGroup()->children(), legendSymbols, legendModel, legendSettings, ctx, image, 0, rasterRenderer, featureRenderer, count );
  return legendSymbols;
}

void HeadlessRender::MapRequest::prepareForRendering( const QSize &outputSize, const QgsRectangle &extent )
{
  mSettings->setOutputSize( outputSize );
  mSettings->setExtent( extent );
  auto expressionContext = createExpressionContext( mSettings );
  expressionContext.lastScope()->addVariable(
    QgsExpressionContextScope::
      StaticVariable( QStringLiteral( "project_ellipsoid" ), mSettings->destinationCrs().ellipsoidAcronym(), true, true )
  );
  mSettings->setExpressionContext( expressionContext );
}

void HeadlessRender::MapRequest::applyRenderSymbols( const RenderSymbols &symbols )
{
  for ( const auto &renderSymbolsItem : symbols )
  {
    auto *layer = mSettings->layers().at( renderSymbolsItem.first );
    if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      if ( !vlayer->renderer() )
        continue;

      const QgsLegendSymbolList symbolList = vlayer->renderer()->legendSymbolItems();

      for ( const auto &item : symbolList )
        vlayer->renderer()->checkLegendSymbolItem( item.ruleKey(), false );

      for ( const auto symbolIndex : renderSymbolsItem.second )
      {
        if ( symbolIndex < 0 || symbolIndex >= symbolList.size() )
          throw QgisHeadlessError( InvalidSymbolIndexError );

        vlayer->renderer()->checkLegendSymbolItem( symbolList.at( symbolIndex ).ruleKey(), true );
      }
    }
    else
      throw QgisHeadlessError( SymbolRenderingNotAdjustableError );
  }
}

void HeadlessRender::setLoggingLevel( HeadlessRender::LogLevel level )
{
  appLogLevel = level;
}
