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

#ifndef TEST_LIB_H
#define TEST_LIB_H

#include <QObject>

class TestLib : public QObject
{
    Q_OBJECT
public:
    TestLib(int argc, char **argv, QObject *parent = nullptr);

private slots:
    void initTestCase();
    void testGetVersion();
    void testRenderVector();
    void testRenderRaster();
    void cleanupTestCase();

private:
    int mArgc;
    char **mArgv;
};

#endif // TEST_LIB_H
