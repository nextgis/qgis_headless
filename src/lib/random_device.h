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

#ifndef QGIS_HEADLESS_RANDOM_DEVICE_H
#define QGIS_HEADLESS_RANDOM_DEVICE_H


namespace HeadlessRender
{
  /**
   * This class represents the seed provider for random numbers generators.
   * You can get identical pseudo-random number sequences by copying another RandomDevice or constructing one from the same seed.
   */
  class QGIS_HEADLESS_EXPORT RandomDevice
  {
    public:
      using SeedType = long long;

    public:
      /**
       * Creates a new RandomDevice with the given seed.
       * \param seed initial value of random device.
       */
      explicit RandomDevice( SeedType seed = 0 ) noexcept;

      /**
       * Returns the seed of device.
       */
      SeedType seed() const noexcept;

    protected:
      SeedType mSeed;
  };

} //namespace HeadlessRender

#endif // QGIS_HEADLESS_RANDOM_DEVICE_H
