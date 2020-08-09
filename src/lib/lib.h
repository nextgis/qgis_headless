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

#include "crs.h"
#include "layer.h"
#include "style.h"
#include "image.h"

#include <tuple>

class QImage;
class QgsMapSettings;

namespace HeadlessRender
{
    typedef std::shared_ptr<QgsMapSettings> QgsMapSettingsPtr;
    typedef std::tuple<double, double, double, double> Extent;
    typedef std::tuple<int, int> Size;

    class QGIS_HEADLESS_EXPORT MapRequest
    {
    public:
        explicit MapRequest();

        void setDpi( int dpi );
        void setSvgPaths( const std::vector< std::string > &paths ); //TODO
        void setCrs( const CRS &crs );
        void addLayer( Layer layer, const Style &style );

        ImagePtr renderImage( const Extent &extent, const Size &size );
        void renderLegend( const Size &size ); //TODO

    private:
        QgsMapSettingsPtr mSettings;
        std::vector<QgsMapLayerPtr> mLayers;
    };

    QGIS_HEADLESS_EXPORT void init( int argc, char **argv );

    QGIS_HEADLESS_EXPORT void deinit();

    QGIS_HEADLESS_EXPORT const char *getVersion();
}

#endif // QGIS_HEADLESS_H
