/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*******************************************************************************
*  Copyright (C) 2023 NextGIS, info@nextgis.ru
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

#include "styleimpl_base.h"

using namespace HeadlessRender;

const QDomDocument &StyleImplBase::data() const
{
    return mData;
}

QDomDocument &StyleImplBase::data()
{
    return mData;
}

QColor StyleImplBase::defaultStyleColor() const
{
    return mDefaultStyleParams.color;
}

bool StyleImplBase::isDefaultStyle() const
{
    return mDefault;
}

UsedAttributes StyleImplBase::usedAttributes() const
{
    return {};
}

StyleFormat StyleImplBase::format() const
{
    return mFormat;
}

StyleImplBase::StyleImplBase( StyleFormat format )
    : mFormat( format )
{

}

StyleImplBase::StyleImplBase( StyleFormat format, const DefaultStyleParams &params )
    : mFormat( format )
    , mDefault( true )
    , mDefaultStyleParams( params )
{

}
