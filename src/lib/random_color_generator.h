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

#ifndef QGIS_HEADLESS_RANDOM_COLOR_GENERATOR_H
#define QGIS_HEADLESS_RANDOM_COLOR_GENERATOR_H

#include <cstddef>
#include <iterator>
#include <random>

#include <QColor>

#include "random_device.h"

namespace HeadlessRender
{
  /**
   * Class for generating the pseudo-random color sequence.
   */
  class RandomColorGenerator
  {
    public:
      /**
       * Random numbers generator, using for colors generating.
       */
      using RandomGeneratorType = std::mt19937;

      class Iterator
      {
        public:
          using value_type = QColor;
          using pointer = QColor *;
          using difference_type = size_t;
          using reference = const QColor &;
          using iterator_category = std::forward_iterator_tag;

        public:
          Iterator( RandomDevice::SeedType seed, size_t index );

          const QColor &operator*() const noexcept;
          void operator++();
          difference_type operator-( const Iterator &other ) const noexcept;
          bool operator!=( const Iterator &other ) const noexcept;

        private:
          void generateColor();

        private:
          RandomGeneratorType mGenerator;
          size_t mIndex;
          QColor mCurrentColor;
      };

    public:
      /**
       * Creates a new RandomColorGenerator, which generates colorsCount colors using the given RandomDevice.
       * \param device RandomDevice providing a seed.
       * \param colorsCount numbers of colors to generate.
       */
      explicit RandomColorGenerator( const RandomDevice &device, size_t colorsCount );

      Iterator begin();
      Iterator end();

    private:
      RandomDevice::SeedType mDeviceSeed;
      size_t mColorsCount;
  };
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_RANDOM_COLOR_GENERATOR_H
