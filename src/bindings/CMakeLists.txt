################################################################################
#  Project:  NextGIS QGIS
#  Purpose:  CMake build scripts
#  Author:   Denis Ilyin, denis.ilyin@nextgis.com
################################################################################
#  Copyright (C) 2020-2023, NextGIS <info@nextgis.com>
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
#   ${PYTHON_LIBRARY}
  ${LIB_NAME}
)

if(UNIX AND NOT OSX_FRAMEWORK)
    add_custom_target(create_symlink
        ALL ${CMAKE_COMMAND} -E create_symlink "$<TARGET_FILE_NAME:${PY_LIB_NAME}>" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/_qgis_headless${CMAKE_SHARED_LIBRARY_SUFFIX}"
        DEPENDS ${PY_LIB_NAME}
    )
    install(FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/_qgis_headless${CMAKE_SHARED_LIBRARY_SUFFIX}" DESTINATION ${PYTHON_SITE_PACKAGES}/qgis_headless COMPONENT python)
endif()

file(READ ${CMAKE_SOURCE_DIR}/qgis_headless/__init__.py INPUT_TEXT)
string(REPLACE "from _qgis_headless" "from ._qgis_headless" INPUT_TEXT "${INPUT_TEXT}")
file(WRITE ${CMAKE_BINARY_DIR}/__init__.py "${INPUT_TEXT}")

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
  install(TARGETS ${PY_LIB_NAME}
    LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES}/qgis_headless
  )
  set(PY_FILES
    ${CMAKE_BINARY_DIR}/__init__.py
    ${CMAKE_SOURCE_DIR}/qgis_headless/script.py
    ${CMAKE_SOURCE_DIR}/qgis_headless/util.py
  )
  install(FILES ${PY_FILES} DESTINATION ${PYTHON_SITE_PACKAGES}/qgis_headless COMPONENT python)
  install(FILES ${CMAKE_SOURCE_DIR}/qgis_headless/_qgis_headless.py.tmplt DESTINATION ${PYTHON_SITE_PACKAGES}/qgis_headless COMPONENT python RENAME _qgis_headless.py)
 
endif()
