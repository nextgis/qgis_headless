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

#ifndef QGIS_HEADLESS_RAWDATA_H
#define QGIS_HEADLESS_RAWDATA_H

#include <QByteArray>
#include <memory>

namespace HeadlessRender
{
    /**
     * Provides an interface for accessing raw data as an array of bytes.
     */
    class QGIS_HEADLESS_EXPORT IRawData
    {
    public:
        virtual ~IRawData(){}

        /**
         * Returns a pointer to the first byte of data.
         */
        virtual const uchar *data() const = 0;

        /**
         * Returns size of data in bytes.
         */
        virtual std::size_t size() const = 0;
    };

    typedef std::shared_ptr<IRawData> RawDataPtr;

    /**
     * Simple implementation of IRawData, based on QByteArray.
     */
    class QGIS_HEADLESS_EXPORT RawData: public IRawData
    {
    public:
        RawData() = default;
        explicit RawData( const QByteArray &byteArray );

        const uchar *data() const override;
        std::size_t size() const override;

    private:
        QByteArray mData;
    };
}

#endif // QGIS_HEADLESS_RAWDATA_H
