###############################################################################
#  Project: NextGIS GIS libraries
#  Purpose: NextGIS headless renderer
#  Author:  Denis Ilyin, denis.ilyin@nextgis.com
###############################################################################
#  Copyright (C) 2020 NextGIS, info@nextgis.ru
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
###############################################################################

import os
import sys
import ctypes
import qgis_headless_py

qgis_headless_py.init([os.path.dirname(os.path.abspath(__file__))])

layer_path = str(sys.argv[1])
style_path = sys.argv[2]
output_path = sys.argv[3]
minx = float(sys.argv[4])
miny = float(sys.argv[5])
maxx = float(sys.argv[6])
maxy = float(sys.argv[7])
width = int(sys.argv[8])
height = int(sys.argv[9])
quality = int(sys.argv[10])

with open(style_path, 'r') as file:
    qmlString = file.read()

image = qgis_headless_py.renderVector(layer_path, qmlString, minx, miny, maxx, maxy, width, height, epsg, quality)

res = (ctypes.c_char * image.size()).from_address(image.data())

with open(output_path, 'wb') as file:
    file.write(bytes(res))

qgis_headless_py.deinit()
