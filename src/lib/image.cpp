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

#include "image.h"

#include <cstdlib>
#include <QByteArray>
#include <QBuffer>
#include <QImage>

HeadlessRender::Image::Image( const QImage &qimage )
{
  mQImage = std::make_shared<QImage>( qimage.convertToFormat( QImage::Format_RGBA8888 ) );
}

const uchar *HeadlessRender::Image::data() const
{
  return mQImage->constBits();
}

std::size_t HeadlessRender::Image::size() const
{
#if QT_VERSION < QT_VERSION_CHECK( 5, 10, 0 )
  return mQImage->byteCount();
#else
  return mQImage->sizeInBytes();
#endif
}

std::pair<int, int> HeadlessRender::Image::sizeWidthHeight() const
{
  return std::make_pair( mQImage->size().width(), mQImage->size().height() );
}
