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

#ifndef QGIS_HEADLESS_EXCEPTIONS_H
#define QGIS_HEADLESS_EXCEPTIONS_H

#include <exception>
#include <QString>

namespace HeadlessRender
{
  class QgisHeadlessError : public std::exception
  {
    public:
      QgisHeadlessError( const QString &message ) : errorMessage( message.toStdString() )
      {}
      const char *what() const throw() override
      {
        return errorMessage.c_str();
      }

    private:
      std::string errorMessage;
  };

  class StyleValidationError : public QgisHeadlessError
  {
    public:
      using QgisHeadlessError::QgisHeadlessError;
  };

  class StyleTypeMismatch : public StyleValidationError
  {
    public:
      using StyleValidationError::StyleValidationError;
  };

  class InvalidLayerSource : public QgisHeadlessError
  {
    public:
      using QgisHeadlessError::QgisHeadlessError;
  };

  class InvalidCRSError : public QgisHeadlessError
  {
    public:
      using QgisHeadlessError::QgisHeadlessError;
  };

  class LayerTypeMismatch : public QgisHeadlessError
  {
    public:
      using QgisHeadlessError::QgisHeadlessError;
  };

  class MemoryLayerError : public QgisHeadlessError
  {
    public:
      using QgisHeadlessError::QgisHeadlessError;
  };

} //namespace HeadlessRender

#endif // QGIS_HEADLESS_EXCEPTIONS_H
