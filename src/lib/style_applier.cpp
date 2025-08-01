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

#include "style_applier.h"

#include <qgsmaplayer.h>
#include <qgspalettedrasterrenderer.h>
#include <qgsrasterlayer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgssymbol.h>
#include <qgsvectorlayer.h>

#include "exceptions.h"
#include "random_color_generator.h"
#include "types.h"

using namespace HeadlessRender;

StyleApplier::StyleApplier( const Style &style ) : mStyle( style )
{}

void StyleApplier::applyStyle( const QgsMapLayerPtr &layer )
{
  if ( mStyle.isDefaultStyle() )
  {
    if ( auto &&vectorLayer = std::dynamic_pointer_cast<QgsVectorLayer>( layer ) )
    {
      setRendererSymbolColor( vectorLayer, mStyle.defaultStyleColor() );
    }
    else if ( auto &&rasterLayer = std::dynamic_pointer_cast<QgsRasterLayer>( layer ) )
    {
      applyDefaultRasterStyle( rasterLayer );
    }
  }
  else
  {
    if ( ( std::dynamic_pointer_cast<QgsVectorLayer>( layer ) && mStyle.type() == DataType::Vector )
         || ( std::dynamic_pointer_cast<QgsRasterLayer>( layer ) && mStyle.type() == DataType::Raster ) )
    {
      QString errorMessage;
      if ( !layer
              ->importNamedStyle( mStyle.data(), errorMessage, static_cast<QgsMapLayer::StyleCategory>( Style::DefaultImportCategories ) ) )
      {
        throw QgisHeadlessError(
          QStringLiteral( "Cannot add style, error message: %1" ).arg( errorMessage )
        );
      }
    }
    else
    {
      throw StyleTypeMismatch( "Layer type and style type do not match" );
    }
  }
}

void StyleApplier::setRendererSymbolColor(
  const std::shared_ptr<QgsVectorLayer> &layer, const QColor &color
)
{
  auto singleRenderer = dynamic_cast< QgsSingleSymbolRenderer * >( layer->renderer() );
  std::unique_ptr<QgsSymbol> newSymbol;

  if ( singleRenderer && singleRenderer->symbol() )
  {
    newSymbol.reset( singleRenderer->symbol()->clone() );
  }

  const QgsSingleSymbolRenderer *embeddedRenderer = nullptr;
  if ( !newSymbol && layer->renderer()->embeddedRenderer() )
  {
    embeddedRenderer = dynamic_cast< const QgsSingleSymbolRenderer * >(
      layer->renderer()->embeddedRenderer()
    );

    if ( embeddedRenderer && embeddedRenderer->symbol() )
    {
      newSymbol.reset( embeddedRenderer->symbol()->clone() );
    }
  }

  if ( newSymbol )
  {
    newSymbol->setColor( color );
    if ( singleRenderer )
    {
      singleRenderer->setSymbol( newSymbol.release() );
    }
    else if ( embeddedRenderer )
    {
      std::unique_ptr<QgsSingleSymbolRenderer> newRenderer( embeddedRenderer->clone() );
      newRenderer->setSymbol( newSymbol.release() );
      layer->renderer()->setEmbeddedRenderer( newRenderer.release() );
    }
  }
}

void StyleApplier::applyDefaultRasterStyle( const std::shared_ptr<QgsRasterLayer> &layer )
{
#if _QGIS_VERSION_INT >= 33000
  // If the default style used for a paletted raster, we need to regenerate the colors with RandomColorGenerator.
  if ( auto &&palettedRendered = dynamic_cast<QgsPalettedRasterRenderer *>( layer->renderer() ) )
  {
    auto classes = palettedRendered->multiValueClasses();
    auto randomColors = RandomColorGenerator( mStyle.getRandomDevice(), classes.length() );

    auto classesIt = classes.begin();
    for ( auto &&randomColor : randomColors )
    {
      classesIt->color = randomColor;
      classesIt++;
    }
    palettedRendered->setMultiValueClasses( classes );
  }
#endif
}
