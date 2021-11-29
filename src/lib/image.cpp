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
    mQImage = std::make_shared<QImage>( qimage.convertToFormat( QImage::Format_RGBA8888 ));
}

const std::string & HeadlessRender::Image::toString()
{
    if ( mData.empty() )
    {
        QByteArray bytes;
        QBuffer buffer( &bytes );

        buffer.open( QIODevice::WriteOnly );
        mQImage->save( &buffer, "TIFF" );
        buffer.close();

        mData = std::string( bytes.constData(), bytes.length() );
    }

    return mData;
}

const unsigned char *HeadlessRender::Image::bits() const
{
    return mQImage->bits();
}

std::size_t HeadlessRender::Image::sizeInBytes() const
{
    return mQImage->sizeInBytes();
}

std::pair<int, int> HeadlessRender::Image::size() const
{
    return std::make_pair( mQImage->size().width(), mQImage->size().height() );
}
