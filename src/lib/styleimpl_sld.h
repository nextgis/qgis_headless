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

#ifndef QGIS_HEADLESS_SLDSTYLE_IMPL_H
#define QGIS_HEADLESS_SLDSTYLE_IMPL_H

#include "styleimpl_base.h"

class QString;
namespace HeadlessRender
{
    class SLDStyleImpl : public StyleImplBase
    {
    public:
        static StyleImplPtr Create( const QString &data );

        DataType type() const override;
        QString exportToString() const override;
        bool importToLayer( QgsMapLayerPtr &layer, QString &errorMessage ) const override;

    private:
        explicit SLDStyleImpl( const QString &data );

        mutable QgsMapLayerPtr mCachedLayer; //for export
    };
}

#endif QGIS_HEADLESS_SLDSTYLE_IMPL_H
