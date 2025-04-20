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

  /**
   * Represents style of layer.
   */
  class QGIS_HEADLESS_EXPORT Style
  {
    public:
      /**
       * Categories of style, imported by default.
       */
      static const StyleCategory DefaultImportCategories;

      /**
       * Creates Style from QML or SLD formatted string.
       * \param string string, containing description of style.
       * \param layerGeometryType type of vector layer's geometry.
       * \param layerType type of layer: raster or vector.
       * \param format format of string, containing descripton of style.
       */
      static Style fromString(
        const std::string &string, const SvgResolverCallback &svgResolverCallback = nullptr,
        LayerGeometryType layerGeometryType = LayerGeometryType::Unknown,
        DataType layerType = DataType::Unknown, StyleFormat format = StyleFormat::QML
      );

      /**
       * Creates Style from QML or SLD file.
       * \param filePath path to file, containing description of style.
       * \param layerGeometryType type of vector layer's geometry.
       * \param layerType type of layer: raster or vector.
       * \param format format of file, containing descripton of style.
       */
      static Style fromFile(
        const std::string &filePath, const SvgResolverCallback &svgResolverCallback = nullptr,
        LayerGeometryType layerGeometryType = LayerGeometryType::Unknown,
        DataType layerType = DataType::Unknown, StyleFormat format = StyleFormat::QML
      );

      /**
       * Creates default Style with given color.
       * \param color color of vector shapes, used only for vector layers.
       * \param layerGeometryType type of vector layer's geometry.
       * \param layerType type of layer: raster or vector.
       * \param format format of style for internal representation.
       */
      static Style fromDefaults(
        const QColor &color, LayerGeometryType layerGeometryType = LayerGeometryType::Unknown,
        DataType layerType = DataType::Unknown
      );

      /**
       * Returns XML representation of style.
       */
      const QDomDocument &data() const;

      /**
       * Provides access to XML representation of style.
       */
      QDomDocument &data();

      /**
       * Returns set of attributes, used in style.
       */
      UsedAttributes usedAttributes() const;

      /**
       * Returns range of scales, within which the layer is visible.
       */
      ScaleRange scaleRange() const;

      /**
       * Returns type of layer's data.
       */
      DataType type() const;

      /**
       * Returns true, if style is default, otherwise returns false.
       * \sa fromDefaults()
       */
      bool isDefaultStyle() const;

      /**
       * Returns the default color for vector layer.
       * \sa fromDefaults()
       */
      QColor defaultStyleColor() const;

      /**
       * Returns XML-string, containing style in given format.
       */
      QString exportToString( StyleFormat format = StyleFormat::QML ) const;

      /**
       * Applies style for given layer. Returns true, if success, otherwise returns false and write reason in errorMessage.
       * \param layer layer to which the style is applied.
       * \param errorMessage reference, where the error message is written.
       * \returns true, if style was applied correctly, overwise returns false.
       */
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
