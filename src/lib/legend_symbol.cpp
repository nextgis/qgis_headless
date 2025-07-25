/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*  Author:  Denis Ilyin, denis.ilyin@nextgis.com
*******************************************************************************
*  Copyright (C) 2022 NextGIS, info@nextgis.ru
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

#include "legend_symbol.h"

using namespace HeadlessRender;

LegendSymbol::LegendSymbol(
  const ImagePtr &icon, const QString &title, const SymbolRender render, const Index index,
  int rasterBand, const bool hasTitle
)
  : mIcon( icon )
  , mTitle( title )
  , mHasTitle( hasTitle )
  , mRender( render )
  , mIndex( index )
  , mRasterBand( rasterBand )
{}

LegendSymbol LegendSymbol::create(
  const ImagePtr &icon, const QString &title, const SymbolRender render, Index index,
  int rasterBand, const bool hasTitle
)
{
  return { icon, title, render, index, std::move( rasterBand ), hasTitle };
}

ImagePtr LegendSymbol::icon() const
{
  return mIcon;
}

QString LegendSymbol::title() const
{
  return mTitle;
}

bool LegendSymbol::hasCategory() const
{
  return mHasCategory;
}

bool LegendSymbol::hasTitle() const
{
  return mHasTitle;
}

void LegendSymbol::setHasCategory( const bool hasCategory )
{
  mHasCategory = hasCategory;
}

LegendSymbol::Index LegendSymbol::index() const
{
  return mIndex;
}

SymbolRender LegendSymbol::render() const
{
  return mRender;
}

int LegendSymbol::rasterBand() const
{
  return mRasterBand;
}
