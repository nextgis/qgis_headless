################################################################################
# Project:  NextGIS QGIS
# Purpose:  CMake build scripts
# Author:   Denis Ilyin, denis.ilyin@nextgis.com
################################################################################
# Copyright (C) 2020, NextGIS <info@nextgis.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
###############################################################################

set(PY_LIB_NAME ${PROJECT_NAME}_py)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pybind11/cmake ${CMAKE_MODULE_PATH})

include(pybind11Tools)
pybind11_add_module(${PY_LIB_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/pylib.cpp
  ${LIB_SOURCES}
)

target_compile_definitions (${PY_LIB_NAME} PRIVATE "QGIS_HEADLESS_EXPORT=${DLLEXPORT}")
target_compile_definitions (${PY_LIB_NAME} INTERFACE "QGIS_HEADLESS_EXPORT=${DLLIMPORT}")

target_include_directories(${PY_LIB_NAME} PRIVATE
  ".."
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
)

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
  install(TARGETS ${PY_LIB_NAME}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  )
endif()
