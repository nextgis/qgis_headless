################################################################################
#  Project:  NextGIS QGIS
#  Purpose:  CMake build scripts
#  Author:   Denis Ilyin, denis.ilyin@nextgis.com
################################################################################
#  Copyright (C) 2020, NextGIS <info@nextgis.com>
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

set(TESTNAME tests)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(${TESTNAME} main.cpp testlib.cpp)
target_include_directories(${TESTNAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/lib")
target_link_libraries(${TESTNAME} Qt5::Test ${LIB_NAME})

target_compile_definitions (${TESTNAME} PRIVATE "QGIS_HEADLESS_EXPORT=${DLLEXPORT}")
target_compile_definitions (${TESTNAME} INTERFACE "QGIS_HEADLESS_EXPORT=${DLLIMPORT}")

add_test(${TESTNAME} ${TESTNAME})

set(TEST_DATA_URL "https://raw.githubusercontent.com/nextgis/testdata/master")

file(DOWNLOAD ${TEST_DATA_URL}/vector/geojson/linez.geojson ${CMAKE_CURRENT_BINARY_DIR}/data/linez.geojson)
file(DOWNLOAD ${TEST_DATA_URL}/raster/geotiff/rgbsmall.tif ${CMAKE_CURRENT_BINARY_DIR}/data/rgbsmall.tif)
file(DOWNLOAD ${TEST_DATA_URL}/style/rgbsmall.qml ${CMAKE_CURRENT_BINARY_DIR}/data/rgbsmall.qml)

add_definitions(-DTEST_DATA_DIR="${CMAKE_CURRENT_BINARY_DIR}/data/")
