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

#ifndef QGIS_HEADLESS_PROJECT_H
#define QGIS_HEADLESS_PROJECT_H

#include "crs.h"
#include "layer.h"
#include <QList>

namespace HeadlessRender
{
  /**
   * Represents project with list of layers and CRS.
   *
   * \warning We can't use more than one Project for now.
   */
  class QGIS_HEADLESS_EXPORT Project
  {
    public:
      /**
       * Read data of project from file.
       * \param filename path to project's file
       * \returns project, loaded from file
       */
      static Project fromFile( const std::string &filename );

      /**
       * Returns current CRS of project.
       */
      CRS crs() const;

      /**
       * Returns all layers in project.
       */
      QList<Layer> layers() const;

    private:
      CRS mCrs;
      QList<Layer> mLayers;
  };
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_PROJECT_H
