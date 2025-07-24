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

#ifndef QGIS_HEADLESS_LEGEND_REQUEST_H
#define QGIS_HEADLESS_LEGEND_REQUEST_H


#include <vector>


#include "layer.h"
#include "legend_symbol.h"
#include "types.h"


class QgsColorRamp;
class QgsFeatureRenderer;
class QgsLayerTreeNode;
class QgsLayerTreeModelLegendNode;
class QgsMultiBandColorRenderer;
class QgsPalettedRasterRenderer;
class QgsRasterRenderer;
class QgsSingleBandGrayRenderer;
class QgsSingleBandPseudoColorRenderer;

namespace HeadlessRender
{
  constexpr int DefaultRasterRenderSymbolCount = 5;

  /**
   * Class for legend rendering.
   */
  class QGIS_HEADLESS_EXPORT LegendRequest final
  {
    public:
      using LegendSymbolsContainer = std::vector<LegendSymbol>;

    public:
      /**
       * Constructs an instance of LegendRequest with default rendering parameters.
       */
      LegendRequest() = default;

      /**
       * Sets the DPI for rendering legend and icons of legend symbols.
       * \param dpi DPI value to be set.
       */
      void setDpi( int dpi ) noexcept;

      /**
       * Sets the number of legend symbols that represent the color ramp. Affects only renderLegendSymbols() method.
       * \param count number of symbols.
       */
      void setColorRampSymbolsCount( size_t count ) noexcept;

      /**
       * Renders the legend for the given layers with the given size.
       * \param layers layers included in the legend.
       * \param size size of the image to be rendered.
       * \returns image containing the rendered legend.
       */
      ImagePtr renderLegend( const std::vector<Layer> &layers, const Size &size = Size() );

      /**
       * Renders the legend symbols representing the given layer.
       * \param layer layer to be represented.
       * \param size size of legend symbols icons.
       * \returns rendered legend symbols.
       */
      LegendSymbolsContainer renderLegendSymbols( const Layer &layer, const Size &size = Size() );

    private:
      using LayerTreeNodeList = QList<QgsLayerTreeNode *>;
      using LayerTreeModelNodeList = QList<QgsLayerTreeModelLegendNode *>;

      class LegendRenderContext;

    private:
      LegendSymbolsContainer renderLayerSymbols( const Layer &layer, LegendRenderContext &context );

      LegendSymbolsContainer renderVectorLayerSymbols(
        const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
        const QgsFeatureRenderer *renderer
      );

      LegendSymbol renderDefaultVectorLayerSymbol(
        QgsLayerTreeModelLegendNode *node, LegendRenderContext &context,
        const QgsFeatureRenderer *renderer
      );

      LegendSymbolsContainer renderRasterLayerSymbols(
        const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
        const QgsRasterRenderer *renderer
      );

      LegendSymbolsContainer renderMultiBandRasterSymbols(
        const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
        const QgsMultiBandColorRenderer *renderer
      );

      LegendSymbolsContainer renderPalettedRasterSymbols(
        const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
        const QgsPalettedRasterRenderer *renderer
      );

      LegendSymbolsContainer renderSingleBandRasterSymbols(
        const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
        const QgsSingleBandGrayRenderer *renderer
      );

      LegendSymbolsContainer renderPseudoColorRasterSymbols(
        const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
        const QgsSingleBandPseudoColorRenderer *renderer
      );

      LegendSymbolsContainer renderDefaultRasterSymbols(
        const LayerTreeModelNodeList &layerNodes, LegendRenderContext &context,
        const QgsRasterRenderer *renderer, int band
      );

      LegendSymbolsContainer renderColorRampSymbols(
        LegendRenderContext &context, const QgsColorRamp &colorRamp, double min, double max,
        int rasterBand = 0
      );

      LegendSymbol renderRasterLayerSymbol( const QImage &image, const QString &title, int rasterBand );
      LegendSymbol renderRasterLayerSymbol(
        QgsLayerTreeModelLegendNode *node, LegendRenderContext &context,
        const QgsRasterRenderer *renderer, int rasterBand
      );

    private:
      int mOutputDpi = 96;
      size_t mColorRampSymbolsCount = DefaultRasterRenderSymbolCount;

      int layerIndex = 0;
  };
} //namespace HeadlessRender


#endif
