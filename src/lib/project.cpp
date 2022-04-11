/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: NextGIS headless renderer
*  Author:  Denis Ilyin, denis.ilyin@nextgis.com
*******************************************************************************
*  Copyright (C) 2022 NextGIS, info@nextgis.ru
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

#include "project.h"

#include <qgsproject.h>
#include "exceptions.h"

void EmptryDeleter(QgsMapLayer *ptr)
{
    //QgsProject itself removes layers
    Q_UNUSED(ptr)
}

HeadlessRender::Project HeadlessRender::Project::fromFile( const std::string &filename )
// Layers stored in QgsProject singleton, so we can't use more than one Project for now
{
    QgsProject *qgsProject = QgsProject::instance();
    bool res = qgsProject->read( QString::fromStdString( filename ));
    if ( !res )
        throw HeadlessRender::QgisHeadlessError( "Unable to open project. Error message: " + QgsProject::instance()->error() );

    Project project;
    project.mCrs = HeadlessRender::CRS::fromWkt( qgsProject->crs().toWkt().toStdString() );

    QVector<QgsMapLayer*> layers = qgsProject->layers<QgsMapLayer *>();
    for ( QgsMapLayer *layer : layers )
    {
        project.mLayers.push_back( HeadlessRender::Layer( HeadlessRender::QgsMapLayerPtr( layer, EmptryDeleter )));
    }

    return project;
}

HeadlessRender::CRS HeadlessRender::Project::crs() const
{
    return mCrs;
}

QList<HeadlessRender::Layer> HeadlessRender::Project::layers() const
{
    return mLayers;
}
