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

#include "crs.h"
#include <qgscoordinatereferencesystem.h>
#include <QString>

static const QString EPSG_3857 = "PROJ: +proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs";
static const QString EPSG_4326 = "PROJ: +proj=longlat +datum=WGS84 +no_def";

HeadlessRender::CRS HeadlessRender::CRS::fromEPSG( long epsg )
{
    CRS crs;

    switch ( epsg )
    {
    case 3857:
        crs.mCRS = std::make_shared<QgsCoordinateReferenceSystem>( EPSG_3857 );
        break;
    case 4326:
        crs.mCRS = std::make_shared<QgsCoordinateReferenceSystem>( EPSG_4326 );
        break;
    default:
        throw std::invalid_argument( "Invalid epsg code" );
        break;
    }

    return crs;
}

HeadlessRender::CRS HeadlessRender::CRS::fromWkt( const std::string &wkt )
{
    CRS crs;
    crs.mCRS = std::make_shared<QgsCoordinateReferenceSystem>( QgsCoordinateReferenceSystem::fromWkt( QString::fromStdString( wkt ) ) );
    return crs;
}

HeadlessRender::QgsCoordinateReferenceSystemPtr HeadlessRender::CRS::qgsCoordinateReferenceSystem() const
{
    return mCRS;
}
