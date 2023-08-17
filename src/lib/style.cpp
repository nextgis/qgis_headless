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

#include "style.h"

#include "styleimpl_qml.h"
#include "styleimpl_sld.h"
#include "exceptions.h"
#include <QFile>

HeadlessRender::Style HeadlessRender::Style::fromString( const std::string &data,
                                                        const SvgResolverCallback &svgResolverCallback /* = nullptr */,
                                                        LayerGeometryType layerGeometryType /* = LayerGeometryType::Undefined */,
                                                        DataType layerType /* = DataType::Unknown */,
                                                        StyleFormat format /* = StyleFormat::QML */)
{
    Style style;
    switch( format )
    {
    case StyleFormat::QML:
        style.mStyleImpl = QMLStyleImpl::Create({
                                                     QString::fromStdString( data ),
                                                     svgResolverCallback,
                                                     layerGeometryType,
                                                     layerType
                                                 });
        break;
    case StyleFormat::SLD:
        style.mStyleImpl = SLDStyleImpl::Create( QString::fromStdString( data ));
        break;
    }

    return style;
}

HeadlessRender::Style HeadlessRender::Style::fromFile( const std::string &filePath,
                                                       const SvgResolverCallback &svgResolverCallback /* = nullptr */,
                                                       LayerGeometryType layerGeometryType /* = LayerGeometryType::Unknown */,
                                                       DataType layerType /* = DataType::Unknown */,
                                                       StyleFormat format /* = StyleFormat::QML */)
{
    std::string data;
    QFile file( QString::fromStdString( filePath) );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QByteArray byteArray = file.readAll();
        data = std::string( byteArray.constData(), byteArray.length() );
    }

    return HeadlessRender::Style::fromString( data, svgResolverCallback, layerGeometryType, layerType, format );
}

HeadlessRender::Style HeadlessRender::Style::fromDefaults( const QColor &color,
                                                           LayerGeometryType layerGeometryType /* = LayerGeometryType::Undefined */,
                                                           DataType layerType /* = DataType::Unknown */,
                                                           StyleFormat format /* = StyleFormat::QML */)
{
    Style style;
    switch( format )
    {
    case StyleFormat::QML:
        style.mStyleImpl = QMLStyleImpl::Create({
                                                    color,
                                                    layerGeometryType,
                                                    layerType
                                                 });
        break;
    case StyleFormat::SLD:
        // Empty
        break;
    }

    return style;
}

const QDomDocument & HeadlessRender::Style::data() const
{
    return mStyleImpl->data();
}

QDomDocument & HeadlessRender::Style::data()
{
    return mStyleImpl->data();
}

HeadlessRender::UsedAttributes HeadlessRender::Style::usedAttributes() const
{
    return mStyleImpl->usedAttributes();
}

HeadlessRender::DataType HeadlessRender::Style::type() const
{
    return mStyleImpl->type();
}

HeadlessRender::StyleFormat HeadlessRender::Style::format() const
{
    return mStyleImpl->format();
}

bool HeadlessRender::Style::isDefaultStyle() const
{
    return mStyleImpl->isDefaultStyle();
}

QColor HeadlessRender::Style::defaultStyleColor() const
{
    return mStyleImpl->defaultStyleColor();
}

QString HeadlessRender::Style::exportToString() const
{
    return mStyleImpl->exportToString();
}

bool HeadlessRender::Style::importToLayer(QgsMapLayerPtr &layer, QString &errorMessage)
{
    return mStyleImpl->importToLayer( layer, errorMessage );
}
