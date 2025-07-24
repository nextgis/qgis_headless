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

#include "legend_request.h"

#include <qgscolorramp.h>
#include <qgslayertree.h>
#include <qgslayertreemodel.h>
#include <qgslayertreenode.h>
#include <qgslegendrenderer.h>
#include <qgslegendsettings.h>
#include <qgsmultibandcolorrenderer.h>
#include <qgspalettedrasterrenderer.h>
#include <qgsrasterlayer.h>
#include <qgsrasterrenderer.h>
#include <qgsrastershader.h>
#include <qgsrendercontext.h>
#include <qgsrenderer.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgssinglebandpseudocolorrenderer.h>
#include <qgsvectorlayer.h>

using namespace HeadlessRender;

namespace RendererType
{
  const auto MULTIBANDCOLOR = QStringLiteral( "multibandcolor" );
  const auto PALETTED = QStringLiteral( "paletted" );
  const auto SINGLEBANDGRAY = QStringLiteral( "singlebandgray" );
  const auto SINGLEBANDPSEUDOCOLOR = QStringLiteral( "singlebandpseudocolor" );

} //namespace RendererType

void LegendRequest::setDpi( int dpi ) noexcept
{
  mOutputDpi = dpi;
}

void LegendRequest::setColorRampSymbolsCount( size_t count ) noexcept
{
  mColorRampSymbolsCount = count;
}

ImagePtr LegendRequest::renderLegend( const std::vector<Layer> &layers, const Size &size )
{
  int width = std::get<0>( size );
  int height = std::get<1>( size );

  QgsLayerTree tree;
  for ( auto &&layer : layers )
  {
    tree.addLayer( layer.qgsMapLayer().get() );
  }

  QgsLayerTreeModel legendModel( &tree );
  QgsLegendRenderer legendRenderer( &legendModel, QgsLegendSettings() );

  qreal dpmm = mOutputDpi / 25.4;

  if ( !width || !height )
  {
    QSizeF minSize = legendRenderer.minimumSize();
    width = minSize.width() * dpmm;
    height = minSize.height() * dpmm;
  }
  QImage img( width, height, QImage::Format_ARGB32_Premultiplied );

  img.fill( Qt::transparent );

  QPainter painter( &img );
  painter.setRenderHint( QPainter::Antialiasing, true );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );

  context.painter()->scale( dpmm, dpmm );

  legendRenderer.drawLegend( context );
  painter.end();

  return std::make_unique<Image>( img );
}

class LegendRequest::LegendRenderContext final
{
  public:
    LegendRenderContext( double dpi, const Size &imageSize );

    LegendRenderContext( const LegendRenderContext &other ) = delete;
    LegendRenderContext( LegendRenderContext &&other ) = delete;

    auto operator=( const LegendRenderContext &other ) = delete;
    auto operator=( LegendRenderContext &&other ) = delete;

    const QImage &drawImage( const QColor &color ) &;
    const QImage &drawImage( QgsLayerTreeModelLegendNode *node ) &;

  private:
    QImage image;
    QPainter painter;
    QgsLegendSettings legendSettings;
    QgsRenderContext qgsRenderContext;
    QgsLayerTreeModelLegendNode::ItemContext itemContext;
};

LegendRequest::LegendRenderContext::LegendRenderContext( double dpi, const Size &imageSize )
  : image( std::get<0>( imageSize ), std::get<1>( imageSize ), QImage::Format_ARGB32_Premultiplied )
  , painter( &image )
  , qgsRenderContext( QgsRenderContext::fromQPainter( &painter ) )
{
  auto width = std::get<0>( imageSize );
  auto height = std::get<1>( imageSize );

  qreal canvasFrac = 0.8;
  qreal dpmm = dpi / 25.4;

  itemContext.context = &qgsRenderContext;
  itemContext.painter = &painter;
  itemContext.top = ( 1 - canvasFrac ) / 2 * height / dpmm;
  itemContext.columnLeft = ( 1 - canvasFrac ) / 2 * width / dpmm;

  qgsRenderContext.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  qgsRenderContext.painter()->scale( dpmm, dpmm );

  legendSettings.setSymbolSize( QSizeF( canvasFrac * width / dpmm, canvasFrac * height / dpmm ) );
  legendSettings.setMaximumSymbolSize( canvasFrac * height / dpmm );
}

const QImage &LegendRequest::LegendRenderContext::drawImage( const QColor &color ) &
{
  image.fill( color );
  return image;
}

const QImage &LegendRequest::LegendRenderContext::drawImage( QgsLayerTreeModelLegendNode *node ) &
{
  image.fill( Qt::transparent );
  node->draw( legendSettings, &itemContext );
  return image;
}

LegendRequest::LegendSymbolsContainer LegendRequest::renderLegendSymbols(
  const Layer &layer, const Size &size
)
{
  LegendRenderContext context( mOutputDpi, size );
  layerIndex = 0;
  return renderLayerSymbols( layer, context );
}

LegendRequest::LegendSymbolsContainer LegendRequest::renderLayerSymbols(
  const Layer &layer, LegendRenderContext &context
)
{
  auto &&qgisLayer = layer.qgsMapLayer();

  QgsLayerTree tree;
  auto &&nodeLayer = tree.addLayer( layer.qgsMapLayer().get() );
  QgsLayerTreeModel model( &tree );

  auto &&nodes = model.layerLegendNodes( nodeLayer );

  if ( auto &&rasterLayer = qobject_cast<const QgsRasterLayer *>( qgisLayer.get() ) )
  {
    return renderRasterLayerSymbols( nodes, context, rasterLayer->renderer() );
  }
  else
  {
    auto &&vectorLayer = qobject_cast<const QgsVectorLayer *>( qgisLayer.get() );
    return renderVectorLayerSymbols( nodes, context, vectorLayer->renderer() );
  }
}

LegendRequest::LegendSymbolsContainer LegendRequest::renderVectorLayerSymbols(
  const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
  const QgsFeatureRenderer *renderer
)
{
  LegendSymbolsContainer symbols;
  symbols.reserve( layerNodes.length() );
  for ( auto &&node : layerNodes )
  {
    symbols.push_back( renderDefaultVectorLayerSymbol( node, context, renderer ) );
  }

  if ( symbols.size() == 1 )
  {
    symbols[0].setHasCategory( false );
  }

  return symbols;
}

LegendSymbol LegendRequest::renderDefaultVectorLayerSymbol(
  QgsLayerTreeModelLegendNode *node, LegendRenderContext &context, const QgsFeatureRenderer *renderer
)
{
  auto symbolRender = HeadlessRender::SymbolRender::Uncheckable;
  if ( renderer->legendSymbolItemsCheckable() )
  {
    symbolRender = node->data( Qt::CheckStateRole ).toBool()
                     ? HeadlessRender::SymbolRender::Checked
                     : HeadlessRender::SymbolRender::Unchecked;
  }

  return HeadlessRender::LegendSymbol::create(
    std::make_shared<HeadlessRender::Image>( context.drawImage( node ) ),
    node->data( Qt::DisplayRole ).toString(), symbolRender, layerIndex++, 0
  );
}

LegendRequest::LegendSymbolsContainer LegendRequest::renderRasterLayerSymbols(
  const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
  const QgsRasterRenderer *renderer
)
{
  if ( renderer->type() == RendererType::MULTIBANDCOLOR )
  {
    return renderMultiBandRasterSymbols( layerNodes, context, static_cast<const QgsMultiBandColorRenderer *>( renderer ) );
  }
  else if ( renderer->type() == RendererType::PALETTED )
  {
    return renderPalettedRasterSymbols( layerNodes, context, static_cast<const QgsPalettedRasterRenderer *>( renderer ) );
  }
  else if ( renderer->type() == RendererType::SINGLEBANDGRAY )
  {
    return renderSingleBandRasterSymbols( layerNodes, context, static_cast<const QgsSingleBandGrayRenderer *>( renderer ) );
  }
  else if ( renderer->type() == RendererType::SINGLEBANDPSEUDOCOLOR )
  {
    return renderPseudoColorRasterSymbols( layerNodes, context, static_cast<const QgsSingleBandPseudoColorRenderer *>( renderer ) );
  }
  else
  {
    return renderDefaultRasterSymbols( layerNodes, context, renderer, 0 );
  }
}

LegendRequest::LegendSymbolsContainer LegendRequest::renderMultiBandRasterSymbols(
  const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
  const QgsMultiBandColorRenderer *renderer
)
{
  LegendSymbolsContainer symbols;

  auto &&usedBands = renderer->usesBands();
  symbols.reserve( layerNodes.length() );
  for ( int i = 0; i < layerNodes.length() && i < usedBands.length(); i++ )
  {
    symbols.push_back(
      renderRasterLayerSymbol( context.drawImage( layerNodes[i] ), QString(), usedBands[i] )
    );
  }
  return symbols;
}

LegendRequest::LegendSymbolsContainer LegendRequest::renderPalettedRasterSymbols(
  const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
  const QgsPalettedRasterRenderer *renderer
)
{
  return renderDefaultRasterSymbols( layerNodes, context, renderer, renderer->band() );
}

LegendRequest::LegendSymbolsContainer LegendRequest::renderSingleBandRasterSymbols(
  const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
  const QgsSingleBandGrayRenderer *renderer
)
{
  QColor first, last;
  if ( renderer->gradient() == QgsSingleBandGrayRenderer::BlackToWhite )
  {
    first = Qt::black;
    last = Qt::white;
  }
  else
  {
    first = Qt::white;
    last = Qt::black;
  }
  QgsGradientColorRamp colorRamp( first, last );

  auto contrastEnhancement = renderer->contrastEnhancement();
  auto min = contrastEnhancement->minimumValue();
  auto max = contrastEnhancement->maximumValue();

  return renderColorRampSymbols( context, colorRamp, min, max, renderer->grayBand() );
}

LegendRequest::LegendSymbolsContainer LegendRequest::renderPseudoColorRasterSymbols(
  const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
  const QgsSingleBandPseudoColorRenderer *renderer
)
{
  auto *rampShader = dynamic_cast<const QgsColorRampShader *>(
    renderer->shader()->rasterShaderFunction()
  );
#if _QGIS_VERSION_INT >= 33800
  if ( rampShader && rampShader->colorRampType() == Qgis::ShaderInterpolationMethod::Linear )
#else
  if ( rampShader && rampShader->colorRampType() == QgsColorRampShader::Type::Interpolated )
#endif
  {
    std::unique_ptr<QgsColorRamp> colorRamp( rampShader->createColorRamp() );
    return renderColorRampSymbols(
      context, *colorRamp, renderer->classificationMin(), renderer->classificationMax(),
      renderer->band()
    );
  }
  else
  {
    return renderDefaultRasterSymbols( layerNodes, context, renderer, renderer->band() );
  }
}

LegendRequest::LegendSymbolsContainer LegendRequest::renderColorRampSymbols(
  LegendRenderContext &context, const QgsColorRamp &colorRamp, double min, double max, int rasterBand
)
{
  auto step = ( max - min ) / ( mColorRampSymbolsCount - 1.0 );
  auto interpStep = 1.0 / ( mColorRampSymbolsCount - 1.0 );

  LegendSymbolsContainer symbols;
  symbols.reserve( mColorRampSymbolsCount );

  for ( size_t i = 0; i < mColorRampSymbolsCount; i++ )
  {
    symbols.push_back(
      renderRasterLayerSymbol( context.drawImage( colorRamp.color( interpStep * i ) ), QString::number( min + step * i ), rasterBand )
    );
  }
  return symbols;
}

LegendSymbol LegendRequest::renderRasterLayerSymbol(
  const QImage &image, const QString &title, int rasterBand
)
{
  return LegendSymbol::create(
    std::make_shared<HeadlessRender::Image>( image ), title, SymbolRender::Uncheckable,
    layerIndex++, rasterBand, !title.isNull()
  );
}

LegendSymbol LegendRequest::renderRasterLayerSymbol(
  QgsLayerTreeModelLegendNode *node, LegendRenderContext &context,
  const QgsRasterRenderer *renderer, int rasterBand
)
{
  return renderRasterLayerSymbol( context.drawImage( node ), node->data( Qt::DisplayRole ).toString(), rasterBand );
}

LegendRequest::LegendSymbolsContainer LegendRequest::renderDefaultRasterSymbols(
  const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
  const QgsRasterRenderer *renderer, int band
)
{
  LegendSymbolsContainer symbols;
  symbols.reserve( layerNodes.length() );
  for ( auto &&node : layerNodes )
  {
    auto &&icon = node->data( Qt::DecorationRole ).value<QIcon>();
    if ( icon.isNull() )
    {
      continue;
    }
    symbols.push_back( renderRasterLayerSymbol( node, context, renderer, band ) );
  }
  return symbols;
}
