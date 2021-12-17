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
#include <QSharedPointer>
#include <QDomDocument>
#include "exceptions.h"
#include "layer.h"

class QgsVectorLayer;
class QgsRenderContext;
class QgsSymbol;

namespace HeadlessRender
{
    class Layer;
    typedef std::function<std::string(const std::string &)> SvgResolverCallback;

    class QGIS_HEADLESS_EXPORT Style
    {
    public:
        typedef long Category;

        Style() = default;
        static Style fromString( const std::string &string, const SvgResolverCallback &svgResolverCallback = nullptr, Layer::GeometryType layerGeometryType = Layer::GeometryType::Unknown  );
        static Style fromFile( const std::string &filePath, const SvgResolverCallback &svgResolverCallback = nullptr, Layer::GeometryType layerGeometryType = Layer::GeometryType::Unknown  );
        std::string data() const;
        std::pair<bool, std::set<std::string>> usedAttributes() const;

        static const Category DefaultImportCategories;

    private:
        static std::string resolveSvgPaths( const std::string &data, const SvgResolverCallback &svgResolverCallback );
        static void resolveSymbol( QgsSymbol *symbol, const SvgResolverCallback &svgResolverCallback );
        static QSharedPointer<QgsVectorLayer> createTemporaryLayerWithStyle( const std::string &style, QString &errorMessage );
        static QSharedPointer<QgsVectorLayer> createTemporaryLayerWithStyle( QDomDocument &style, QString &errorMessage );
        static bool validateGeometryType( const std::string &style, Layer::GeometryType layerGeometryType );
        static bool validateStyle( const std::string &style, QString &errorMessage );
        static void removeLayerGeometryTypeElement( QDomDocument & );
        QSet<QString> referencedFields( const QSharedPointer<QgsVectorLayer> &layer, const QgsRenderContext &context, const QString &providerId ) const;

        std::string mData;
    };
}

#endif // QGIS_HEADLESS_STYLE_H
