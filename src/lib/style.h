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

#ifndef QGIS_HEADLESS_STYLE_H
#define QGIS_HEADLESS_STYLE_H

#include <string>
#include <functional>
#include <set>
#include <QSet>
#include <QString>
#include <QDomDocument>
#include <QColor>
#include "exceptions.h"
#include "layer.h"
#include "types.h"

class QgsVectorLayer;
class QgsRasterLayer;
class QgsRenderContext;
class QgsSymbol;
class QgsVectorLayer;
class QgsRasterLayer;

namespace HeadlessRender
{
    class Layer;
    typedef std::function<std::string(const std::string &)> SvgResolverCallback;
    typedef std::pair<bool, std::set<std::string>> UsedAttributes;
    typedef std::shared_ptr<QgsVectorLayer> QgsVectorLayerPtr;
    typedef std::shared_ptr<QgsRasterLayer> QgsRasterLayerPtr;

    class QGIS_HEADLESS_EXPORT Style
    {
    public:
        typedef long Category;

        Style() = default;

        static Style fromString( const std::string &string, const SvgResolverCallback &svgResolverCallback = nullptr, Layer::GeometryType layerGeometryType = Layer::GeometryType::Unknown, DataType layerType = DataType::Unknown );
        static Style fromFile( const std::string &filePath, const SvgResolverCallback &svgResolverCallback = nullptr, Layer::GeometryType layerGeometryType = Layer::GeometryType::Unknown, DataType layerType = DataType::Unknown );
        static Style fromDefaults( const QColor &color, Layer::GeometryType layerGeometryType = Layer::GeometryType::Unknown, DataType layerType = DataType::Unknown );

        QDomDocument data() const;
        HeadlessRender::UsedAttributes usedAttributes() const;
        DataType type() const;

        bool isDefaultStyle() const;
        QColor defaultStyleColor() const;

        QString exportToQML() const;

        static const Category DefaultImportCategories;
        static bool importToLayer( QgsMapLayerPtr &layer, QDomDocument &styleData, QString &errorMessage );

    private:
        bool validateGeometryType( Layer::GeometryType layerGeometryType ) const;
        void removeLayerGeometryTypeElement( QDomDocument & ) const;
        void resolveSymbol( QgsSymbol *symbol, const SvgResolverCallback &svgResolverCallback ) const;
        QDomDocument resolveSvgPaths( const SvgResolverCallback &svgResolverCallback ) const;
        bool validateStyle( QString &errorMessage ) const;
        QgsVectorLayerPtr createTemporaryVectorLayerWithStyle( QString &errorMessage ) const;
        QgsRasterLayerPtr createTemporaryRasterLayerWithStyle( QString &errorMessage ) const;
        QSet<QString> referencedFields( const QgsVectorLayerPtr &layer, const QgsRenderContext &context, const QString &providerId ) const;
        HeadlessRender::UsedAttributes readUsedAttributes() const;

        QDomDocument mData;
        mutable DataType mType = DataType::Unknown;

        struct DefaultStyleParams
        {
            QColor color;
            Layer::GeometryType layerGeometryType;
            DataType layerType;
        };

        DefaultStyleParams mDefaultStyleParams;
        bool mDefault = false;

        mutable HeadlessRender::UsedAttributes mUsedAttributesCache;
        mutable bool mUsedAttributesCached = false;

        mutable QgsMapLayerPtr mCachedTemporaryLayer;

        friend class Layer;
    };
}

#endif // QGIS_HEADLESS_STYLE_H
