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
      /**
       * Initializes the MapRequest object with default settings.
       */
      explicit MapRequest();

      /**
       * Sets the DPI for rendering.
       * \param dpi the dots per inch to set for rendering.
       */
      void setDpi( int dpi );

      /**
       * Sets the coordinate reference system (CRS) for rendering.
       * \param crs the CRS to set for rendering.
       */
      void setCrs( const CRS &crs );

      /**
       * Adds a layer with a given style and label.
       * \param layer the layer to add.
       * \param style the style to apply to the layer.
       * \param label the label for the layer (optional).
       * \returns the index of the added layer.
       */
      LayerIndex addLayer( Layer &layer, Style &style, const std::string &label = "" );

      /**
       * Adds a project with multiple layers.
       * \param project the project containing layers to add.
       */
      void addProject( const Project &project );

      /**
       * Renders an image based on the given extent and size.
       * \param extent the geographic extent for rendering.
       * \param size the size of the output image.
       * \param symbols optional render symbols to apply.
       * \returns a shared pointer to the rendered image.
       */
      ImagePtr renderImage( const Extent &extent, const Size &size, const RenderSymbols &symbols = {} );

      /**
       * Renders a legend based on the given size.
       * \param size the size of the output legend image (optional).
       * \returns a shared pointer to the rendered legend image.
       */
      ImagePtr renderLegend( const Size &size = Size() );

      /**
       * Exports the map to a PDF file with the given extent and size.
       * \param filepath the path to save the exported PDF file.
       * \param extent the geographic extent for rendering.
       * \param size the size of the output PDF.
       */
      void exportPdf( const std::string &filepath, const Extent &extent, const Size &size );

      /**
       * Retrieves legend symbols for a specific layer index.
       * \param index the index of the layer to retrieve legend symbols for.
       * \param size the size of the legend symbol images (optional).
       * \param count the number of legend symbols to generate (optional).
       * \returns a vector of legend symbols.
       */
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

  /**
   * Initializes the QGIS headless environment with command line arguments.
   * \param argc the number of command line arguments.
   * \param argv an array of command line argument strings.
   */
  QGIS_HEADLESS_EXPORT void init( int argc, char **argv );

  /**
   * Deinitializes the QGIS headless environment.
   */
  QGIS_HEADLESS_EXPORT void deinit();

  /**
   * Sets the paths to search for SVG files.
   * \param paths a vector of paths where SVG files can be located.
   */
  QGIS_HEADLESS_EXPORT void setSvgPaths( const std::vector<std::string> &paths );

  /**
   * Retrieves the current list of SVG file paths.
   * \returns a vector containing the paths where SVG files are searched for.
   */
  QGIS_HEADLESS_EXPORT std::vector<std::string> getSvgPaths();

  /**
   * Gets the version of the headless library as a string.
   * \returns a C-style string representing the version of the headless library.
   */
  QGIS_HEADLESS_EXPORT const char *getVersion();

  /**
   * Retrieves the version of QGIS used in the headless environment as a string.
   * \returns a C-style string representing the QGIS version.
   */
  QGIS_HEADLESS_EXPORT const char *getQGISVersion();

  /**
   * Gets the version of QGIS as an integer value.
   * \returns an integer representing the QGIS version.
   */
  QGIS_HEADLESS_EXPORT int getQGISVersionInt();

  /**
   * Sets the logging level for the headless environment.
   * \param level the desired log level to set.
   */
  QGIS_HEADLESS_EXPORT void setLoggingLevel( HeadlessRender::LogLevel level );
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_H
