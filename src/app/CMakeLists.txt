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

set(APP_NAME ${PROJECT_NAME}_app)

add_executable(${APP_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp)
target_include_directories(${APP_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/src/lib
)
target_link_libraries(${APP_NAME} ${LIB_NAME})

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
  install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${INSTALL_BIN_DIR})
endif()
