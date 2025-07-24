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

  mLayers.push_back( layer );

  QList<QgsMapLayer *> qgsMapLayers;
  for ( auto &&layer : mLayers )
    qgsMapLayers.push_back( layer.qgsMapLayer().get() );
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
    mLayers.push_back( layer );

  QList<QgsMapLayer *> qgsMapLayers;
  for ( auto &&layer : mLayers )
    qgsMapLayers.push_back( layer.qgsMapLayer().get() );
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
  LegendRequest legendRequest;
  legendRequest.setDpi( mSettings->outputDpi() );
  return legendRequest.renderLegend( mLayers, size );
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

std::vector<HeadlessRender::LegendSymbol> HeadlessRender::MapRequest::legendSymbols(
  const LayerIndex index, const HeadlessRender::Size &size /* = Size() */,
  int count /* = DefaultRasterRenderSymbolCount */
)
{
  if ( mLayers.size() <= index )
    throw QgisHeadlessError( InvalidLayerIndexError );

  LegendRequest legendRequest;
  legendRequest.setDpi( mSettings->outputDpi() );
  legendRequest.setColorRampSymbolsCount( count );
  return legendRequest.renderLegendSymbols( mLayers.at( index ), size );
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
