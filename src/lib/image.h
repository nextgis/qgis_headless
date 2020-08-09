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

#ifndef QGIS_HEADLESS_IMAGE_H
#define QGIS_HEADLESS_IMAGE_H

#include <memory>
#include <string>

class QImage;

namespace HeadlessRender
{
    typedef std::shared_ptr<QImage> QImagePtr;

    class QGIS_HEADLESS_EXPORT Image
    {
    public:
        Image() = default;
        Image(const QImage &qimage);
        std::string toString() const;
    private:
        std::shared_ptr<QImage> mQImage;
    };

    typedef std::shared_ptr<Image> ImagePtr;
}

#endif // QGIS_HEADLESS_IMAGE_H
