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
#include <QString>

namespace HeadlessRender
{
    class LegendSymbol;
    using LegendSymbolPtr = std::shared_ptr<LegendSymbol>;

    class LegendSymbol
    {
    public:
        struct RasterBand
        {
            explicit RasterBand() = default;
            RasterBand( int red, int green, int blue, int alpha );

            int red() const;
            int green() const;
            int blue() const;
            int alpha() const;

        private:
            int mRed = 0;
            int mGreen = 0;
            int mBlue = 0;
            int mAlpha = 0;
        };

        using Index = int;

        static LegendSymbol create( const ImagePtr icon, const QString &title, bool isEnabled, Index index, RasterBand rasterBand );

        ImagePtr icon() const;
        QString title() const;
        bool hasCategory() const;
        void setHasCategory( bool hasCategory );
        Index index() const;
        bool isEnabled() const;
        RasterBand rasterBand() const;

    private:
        LegendSymbol( const ImagePtr icon, const QString &title, bool isEnabled, Index index, RasterBand rasterBand );

        ImagePtr mIcon;
        QString mTitle;
        bool mHasCategory = true;
        bool mIsEnabled;
        Index mIndex = 0;
        RasterBand mRasterBand;
    };
}

#endif // QGIS_HEADLESS_LEGEND_SYMBOL_H
