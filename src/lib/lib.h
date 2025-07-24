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

#ifndef QGIS_HEADLESS_H
#define QGIS_HEADLESS_H

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

#include "crs.h"
#include "layer.h"
#include "style.h"
#include "image.h"
#include "legend_request.h"
#include "legend_symbol.h"
#include "raw_data.h"
#include "project.h"

class QgsRectangle;

namespace HeadlessRender
{
  using SymbolIndexVector = std::vector<LegendSymbol::Index>;
  using RenderSymbols = std::unordered_map<LayerIndex, SymbolIndexVector>;

  class QGIS_HEADLESS_EXPORT MapRequest
  {
    public:
      explicit MapRequest();

      void setDpi( int dpi );
      void setCrs( const CRS &crs );
      LayerIndex addLayer( Layer &layer, Style &style, const std::string &label = "" );
      void addProject( const Project &project );

      ImagePtr renderImage( const Extent &extent, const Size &size, const RenderSymbols &symbols = {} );
      ImagePtr renderLegend( const Size &size = Size() );
      void exportPdf( const std::string &filepath, const Extent &extent, const Size &size );

      std::vector<LegendSymbol> legendSymbols(
        LayerIndex index, const Size &size = Size(), int count = DefaultRasterRenderSymbolCount
      );

    protected:
      /**
       * Prepares mSettings for rendering
       * \param outputSize size of the rendered image
       * \param extent extent for rendering
       */
      void prepareForRendering( const QSize &outputSize, const QgsRectangle &extent );

    private:
      void applyRenderSymbols( const RenderSymbols &symbols );

      QgsMapSettingsPtr mSettings;
      QgsLayerTreePtr mQgsLayerTree;
      std::vector<Layer> mLayers;
      RenderSymbols mDefaultRenderSymbols;
  };

  QGIS_HEADLESS_EXPORT void init( int argc, char **argv );

  QGIS_HEADLESS_EXPORT void deinit();

  QGIS_HEADLESS_EXPORT void setSvgPaths( const std::vector<std::string> &paths );

  QGIS_HEADLESS_EXPORT std::vector<std::string> getSvgPaths();

  QGIS_HEADLESS_EXPORT const char *getVersion();

  QGIS_HEADLESS_EXPORT const char *getQGISVersion();

  QGIS_HEADLESS_EXPORT int getQGISVersionInt();

  QGIS_HEADLESS_EXPORT void setLoggingLevel( HeadlessRender::LogLevel level );
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_H
