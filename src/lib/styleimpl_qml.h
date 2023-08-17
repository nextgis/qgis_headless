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

#ifndef QGIS_HEADLESS_QMLSTYLE_IMPL_H
#define QGIS_HEADLESS_QMLSTYLE_IMPL_H

#include "styleimpl_base.h"

#include <QString>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgssymbol.h>

namespace HeadlessRender
{
    typedef std::function<std::string(const std::string &)> SvgResolverCallback;
    typedef std::shared_ptr<QgsVectorLayer> QgsVectorLayerPtr;
    typedef std::shared_ptr<QgsRasterLayer> QgsRasterLayerPtr;

    class QMLStyleImpl : public StyleImplBase
    {
    public:
        static const StyleCategory DefaultImportCategories;

        struct CreateParams
        {
            QString data;
            SvgResolverCallback callback;
            LayerGeometryType layerGeometry;
            DataType layerType;
        };

        static StyleImplPtr Create( const CreateParams &params );
        static StyleImplPtr Create( const DefaultStyleParams &params );

        UsedAttributes usedAttributes() const override;
        bool isDefaultStyle() const override;

        DataType type() const override;
        bool importToLayer( QgsMapLayerPtr &layer, QString &errorMessage ) const override;
        QString exportToString() const override;

    private:
        explicit QMLStyleImpl( const QString &params );
        explicit QMLStyleImpl( const DefaultStyleParams &params );

        void init( const CreateParams &params );

        bool importToLayer( QgsMapLayerPtr &layer, QDomDocument style, QString &errorMessage ) const;
        bool validateGeometryType( LayerGeometryType layerGeometryType ) const;
        bool validateStyle( QString &errorMessage ) const;
        void removeLayerGeometryTypeElement( QDomDocument & ) const;
        void resolveSymbol( QgsSymbol *symbol, const SvgResolverCallback &svgResolverCallback ) const;
        QDomDocument resolveSvgPaths( const SvgResolverCallback &svgResolverCallback ) const;
        QgsVectorLayerPtr createTemporaryVectorLayerWithStyle( QString &errorMessage ) const;
        QgsRasterLayerPtr createTemporaryRasterLayerWithStyle( QString &errorMessage ) const;
        QSet<QString> referencedFields( const QgsVectorLayerPtr &layer, const QgsRenderContext &context, const QString &providerId ) const;
        UsedAttributes readUsedAttributes() const;
        bool hasEnabledDiagrams( const QgsVectorLayerPtr &layer ) const;

        mutable DataType mType = DataType::Unknown;

        mutable UsedAttributes mUsedAttributesCache;
        mutable bool mUsedAttributesCached = false;

        mutable QgsMapLayerPtr mCachedTemporaryLayer;

    };
}

#endif QGIS_HEADLESS_QMLSTYLE_IMPL_H
