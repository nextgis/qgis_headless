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
#include "raw_data.h"

class QImage;

namespace HeadlessRender
{
  typedef std::shared_ptr<QImage> QImagePtr;

  /**
   * This class represents an image, based on QImage.
   */
  class QGIS_HEADLESS_EXPORT Image : public IRawData
  {
    public:
      /**
       * Constructs a null image.
       */
      Image() = default;

      /**
       * Constructs image from a given QImage.
       * \param qimage QImage to be copied.
       */
      explicit Image( const QImage &qimage );

      /**
       * Returns size of image.
       * \returns std::pair, where .first is width and .second is height.
       */
      std::pair<int, int> sizeWidthHeight() const;

      /**
       * Returns a pointer to the first pixel data.
       */
      const uchar *data() const override;

      /**
       * Returns size of pixel data in bytes.
       */
      std::size_t size() const override;

    private:
      QImagePtr mQImage;
  };

  typedef std::shared_ptr<Image> ImagePtr;
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_IMAGE_H
