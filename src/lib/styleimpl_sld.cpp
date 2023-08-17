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

#include "styleimpl_sld.h"

#include <qgsvectorlayer.h>
#include <QString>
#include <QFile>

#include "utils.h"
#include "exceptions.h"

namespace HeadlessRender
{
    namespace TAGS
    {
        static const QString StyledLayerDescriptor = "StyledLayerDescriptor";
        static const QString NamedLayer = "NamedLayer";
    }

    namespace ErrorStrings
    {
        static const QString ExportFailed = "Export failed: cannot create temporary layer";
    }
}

using namespace HeadlessRender;

StyleImplPtr SLDStyleImpl::Create( const QString &data )
{
    return StyleImplPtr( new SLDStyleImpl( data ));
}

DataType SLDStyleImpl::type() const
{
    return DataType::Unknown;
}

QString SLDStyleImpl::exportToString() const
{
    auto doExport = []( const QgsMapLayerPtr &qgsMapLayer )
    {
        QDomDocument exportedStyle;
        QString errorMessage;
        qgsMapLayer->exportSldStyle( exportedStyle, errorMessage);
        return exportedStyle.toString();
    };

    if ( mCachedLayer )
    {
        return doExport( mCachedLayer );
    }
    else
    {
        QgsMapLayerPtr qgsMapLayer = createTemporaryVectorLayer( QgsVectorLayer::LayerOptions() );
        if ( !qgsMapLayer )
        {
            throw QgisHeadlessError( ErrorStrings::ExportFailed );
        }
        return doExport( qgsMapLayer );
    }
}

bool SLDStyleImpl::importToLayer(QgsMapLayerPtr &layer, QString &errorMessage) const
{
    const QDomElement myRoot = mData.firstChildElement( TAGS::StyledLayerDescriptor );
    const QDomElement namedLayerElem = myRoot.firstChildElement( TAGS::NamedLayer );

    bool result = layer->readSld( namedLayerElem, errorMessage );
    if ( result && !mCachedLayer )
    {
        mCachedLayer.reset( layer->clone() );
    }
    return result;
}

SLDStyleImpl::SLDStyleImpl(const QString &data)
    : StyleImplBase( StyleFormat::SLD )
{
    mData.setContent( data, true );
}
