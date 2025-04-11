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
#include <memory>
#include <QString>
#include <QColor>
#include <QDomDocument>

#include "types.h"

class QDomDocument;
class QgsVectorLayer;
class QgsRasterLayer;
class QgsSymbol;
class QgsRenderContext;

namespace HeadlessRender
{
  typedef std::function<std::string( const std::string & )> SvgResolverCallback;
  typedef std::shared_ptr<QgsVectorLayer> QgsVectorLayerPtr;
  typedef std::shared_ptr<QgsRasterLayer> QgsRasterLayerPtr;

  class QGIS_HEADLESS_EXPORT Style
  {
    public:
      static const StyleCategory DefaultImportCategories;

      static Style fromString(
        const std::string &string, const SvgResolverCallback &svgResolverCallback = nullptr,
        LayerGeometryType layerGeometryType = LayerGeometryType::Unknown,
        DataType layerType = DataType::Unknown, StyleFormat format = StyleFormat::QML
      );

      static Style fromFile(
        const std::string &filePath, const SvgResolverCallback &svgResolverCallback = nullptr,
        LayerGeometryType layerGeometryType = LayerGeometryType::Unknown,
        DataType layerType = DataType::Unknown, StyleFormat format = StyleFormat::QML
      );

      static Style fromDefaults(
        const QColor &color, LayerGeometryType layerGeometryType = LayerGeometryType::Unknown,
        DataType layerType = DataType::Unknown, StyleFormat format = StyleFormat::QML
      );

      const QDomDocument &data() const;
      QDomDocument &data();
      UsedAttributes usedAttributes() const;
      ScaleRange scaleRange() const;
      DataType type() const;

      bool isDefaultStyle() const;
      QColor defaultStyleColor() const;

      QString exportToString( StyleFormat format = StyleFormat::QML ) const;

      bool importToLayer( QgsMapLayerPtr &layer, QString &errorMessage ) const;

    private:
      struct CreateParams
      {
          QString data;
          SvgResolverCallback callback;
          LayerGeometryType layerGeometry;
          DataType layerType;
      };

      struct DefaultStyleParams
      {
          QColor color;
          LayerGeometryType layerGeometryType;
          DataType layerType;
      };

      Style() = default;

      void init( const CreateParams &params );
      void init( const DefaultStyleParams &params );

      bool importToLayer( QgsMapLayerPtr &layer, QDomDocument style, QString &errorMessage ) const;
      bool validateGeometryType( LayerGeometryType layerGeometryType ) const;
      bool validateStyle( QString &errorMessage ) const;
      void removeLayerGeometryTypeElement( QDomDocument & ) const;
      void resolveSymbol( QgsSymbol *symbol, const SvgResolverCallback &svgResolverCallback ) const;
      QDomDocument resolveSvgPaths( const SvgResolverCallback &svgResolverCallback ) const;
      QgsVectorLayerPtr createTemporaryVectorLayerWithStyle( QString &errorMessage ) const;
      QgsRasterLayerPtr createTemporaryRasterLayerWithStyle( QString &errorMessage ) const;
      QgsMapLayerPtr createTemporaryLayerWithStyleByType( DataType type, QString &errorMessage ) const;
      QSet<QString> referencedFields(
        const QgsVectorLayerPtr &layer, const QgsRenderContext &context, const QString &providerId
      ) const;
      UsedAttributes readUsedAttributes() const;
      bool hasEnabledDiagrams( const QgsVectorLayerPtr &layer ) const;

      QDomDocument mData;

      bool mDefault = false;
      DefaultStyleParams mDefaultStyleParams;

      mutable DataType mType = DataType::Unknown;

      mutable UsedAttributes mUsedAttributesCache;
      mutable bool mUsedAttributesCached = false;

      mutable ScaleRange mScaleRange = { -1, 0 }; // "-1" - not cached; "-2" has no scale range

      mutable QgsMapLayerPtr mCachedTemporaryLayer;
  };
} //namespace HeadlessRender

#endif // QGIS_HEADLESS_STYLE_H
