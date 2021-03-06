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

set(PY_LIB_NAME _${PROJECT_NAME})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pybind11/cmake ${CMAKE_MODULE_PATH})

include(pybind11Tools)
pybind11_add_module(${PY_LIB_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/pylib.cpp
  ${LIB_SOURCES}
)

target_include_directories(${PY_LIB_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/src/lib
  ${PYTHON_INCLUDE_DIRS}
  ${QGIS_CORE_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/pybind11/include
)

target_link_libraries(${PY_LIB_NAME} PRIVATE
  Qt5::Core
  Qt5::Xml
  Qt5::Gui
  Qt5::Widgets
  Qt5::Network
  ${PYTHON_LIBRARY}
  ${QGIS_CORE_LIBRARIES}
  ${LIB_NAME}
)

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
  install(TARGETS ${PY_LIB_NAME}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  )
endif()
