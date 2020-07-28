/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*  Author:  Denis Ilyin, denis.ilyin@nextgis.com
*******************************************************************************
*  Copyright (C) 2020 NextGIS, info@nextgis.ru
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 2 of the License, or
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

namespace HeadlessRender
{
    class Image
    {
    public:
        Image() = default;
        Image(unsigned char *data, int size);
        ~Image();

        unsigned char *getData() { return mData; }
        int getSize() const { return mSize; }

    private:
        unsigned char *mData = nullptr;
        int mSize = 0;
    };

    QGIS_HEADLESS_EXPORT void init(int argc, char **argv);

    QGIS_HEADLESS_EXPORT void deinit();

    QGIS_HEADLESS_EXPORT std::shared_ptr<Image> renderVector(const char *uri,
                                                             const char *qmlString,
                                                             double minx,
                                                             double miny,
                                                             double maxx,
                                                             double maxy,
                                                             int width,
                                                             int height,
                                                             int epsg,
                                                             int quality = -1);

    QGIS_HEADLESS_EXPORT std::shared_ptr<Image> renderRaster(const char *uri,
                                                             const char *qmlString,
                                                             double minx,
                                                             double miny,
                                                             double maxx,
                                                             double maxy,
                                                             int width,
                                                             int height,
                                                             int epsg,
                                                             int quality = -1);

    QGIS_HEADLESS_EXPORT const char *getVersion();
}

#endif // QGIS_HEADLESS_H
