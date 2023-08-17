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

#ifndef QGIS_HEADLESS_STYLE_IMPL_H
#define QGIS_HEADLESS_STYLE_IMPL_H

#include <memory>
#include <set>

#include <QDomDocument>
#include <QColor>
#include <QString>

#include "types.h"

class QgsMapLayer;

namespace HeadlessRender
{
    class StyleImplBase
    {
    public:
        virtual ~StyleImplBase() = default;

        virtual const QDomDocument & data() const;
        virtual QDomDocument &data();
        virtual QColor defaultStyleColor() const;
        virtual bool isDefaultStyle() const;

        virtual UsedAttributes usedAttributes() const;
        virtual DataType type() const = 0;
        virtual QString exportToString() const = 0;
        virtual bool importToLayer( QgsMapLayerPtr &layer, QString &errorMessage ) const = 0;

        StyleFormat format() const;

    protected:
        struct DefaultStyleParams
        {
            QColor color;
            LayerGeometryType layerGeometryType;
            DataType layerType;
        };

        explicit StyleImplBase( StyleFormat );
        StyleImplBase( StyleFormat, const DefaultStyleParams &params );

        QDomDocument mData;

        StyleFormat mFormat;
        bool mDefault = false;
        DefaultStyleParams mDefaultStyleParams;
    };


    typedef std::shared_ptr<StyleImplBase> StyleImplPtr;
}

#endif QGIS_HEADLESS_STYLE_IMPL_H
