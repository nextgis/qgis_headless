/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*  Author:  Denis Ilyin, denis.ilyin@nextgis.com
*******************************************************************************
*  Copyright (C) 2021 NextGIS, info@nextgis.ru
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

#ifndef QGIS_HEADLESS_LEGEND_SYMBOL_H
#define QGIS_HEADLESS_LEGEND_SYMBOL_H

#include "image.h"
#include "types.h"
#include <QString>

namespace HeadlessRender
{
    class LegendSymbol;
    using LegendSymbolPtr = std::shared_ptr<LegendSymbol>;

    /**
     * Represents symbol of layer in map's legend
     */
    class LegendSymbol
    {
    public:
        using Index = int;

        /**
         * Returns LegendSymbol, created with given icon, title and layer's index.
         */
        static LegendSymbol create( const ImagePtr icon, const QString &title, SymbolRender render, Index index, int rasterBand, bool hasTitle = true );

        /**
         * Returns icon of symbol.
         */
        ImagePtr icon() const;

        /**
         * Returns title of symbol.
         */
        QString title() const;

        /**
         * Returns true if the layer has a category, i.e. it is part of some group, otherwise returns false.
         */
        bool hasCategory() const;

        /**
         * Returns true if layer has a title, otherwise returns false.
         */
        bool hasTitle() const;

        /**
         * Sets whether the layer has a category or not, depending on the value of the hasCategory parameter.
         * \sa hasCategory()
         */
        void setHasCategory( bool hasCategory );

        /**
         * Returns index of layer.
         */
        Index index() const;

        /**
         * Returns render type of symbol.
         */
        SymbolRender render() const;

        /**
         * Return raster band number
         */
        int rasterBand() const;

    private:
        LegendSymbol( const ImagePtr icon, const QString &title, SymbolRender render, Index index, int rasterBand, bool hasTitle = true );

        ImagePtr mIcon;
        QString mTitle;
        bool mHasCategory = true;
        bool mHasTitle = false;
        SymbolRender mRender = SymbolRender::Uncheckable;
        Index mIndex = 0;
        int mRasterBand;
    };
}

#endif // QGIS_HEADLESS_LEGEND_SYMBOL_H
