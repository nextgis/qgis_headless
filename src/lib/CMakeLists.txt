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

set(LIB_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/lib.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/crs.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/layer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/style.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/legend_symbol.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
)

set(LIB_PRIVATE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
)

set(LIB_PUBLIC_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/lib.h
  ${CMAKE_CURRENT_SOURCE_DIR}/crs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/layer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/style.h
  ${CMAKE_CURRENT_SOURCE_DIR}/legend_symbol.h
  ${CMAKE_CURRENT_SOURCE_DIR}/exceptions.h
)

add_library(${LIB_NAME} ${LIB_SOURCES} ${LIB_PUBLIC_HEADERS} ${LIB_PRIVATE_HEADERS})

target_include_directories(${LIB_NAME} PRIVATE ${QGIS_CORE_INCLUDE_DIRS})

target_link_libraries(${LIB_NAME}
  Qt5::Core
  Qt5::Xml
  Qt5::Gui
  Qt5::Widgets
  Qt5::Network
  ${QGIS_CORE_LIBRARIES}
)

target_compile_definitions (${LIB_NAME} PRIVATE "QGIS_HEADLESS_EXPORT=${DLLEXPORT}")
target_compile_definitions (${LIB_NAME} INTERFACE "QGIS_HEADLESS_EXPORT=${DLLIMPORT}")

add_definitions(-D_USE_MATH_DEFINES)

# Add path to includes to build-tree export
target_include_directories(${LIB_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL)
    install(FILES ${LIB_PUBLIC_HEADERS} DESTINATION "${INSTALL_INC_DIR}" COMPONENT headers)
endif()

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
  install(TARGETS ${LIB_NAME}
    EXPORT ${PACKAGE_UPPER_NAME}Targets
    RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT libraries
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR} COMPONENT libraries
    LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT libraries
    INCLUDES DESTINATION ${INSTALL_INC_DIR}
  )

  # Add all targets to the build-tree export set
  export(TARGETS ${LIB_NAME}
      FILE ${PROJECT_BINARY_DIR}/${PACKAGE_UPPER_NAME}Targets.cmake)

  # Create the <Package>Config.cmake file
  configure_file(${PROJECT_SOURCE_DIR}/cmake/PackageConfig.cmake.in
      ${PROJECT_BINARY_DIR}/${PACKAGE_UPPER_NAME}Config.cmake @ONLY)

  # Install the <Package>Config.cmake
  install(FILES
    ${PROJECT_BINARY_DIR}/${PACKAGE_UPPER_NAME}Config.cmake
    DESTINATION ${INSTALL_CMAKECONF_DIR} COMPONENT dev)

  # Install the export set for use with the install-tree
  install(EXPORT ${PACKAGE_UPPER_NAME}Targets DESTINATION ${INSTALL_CMAKECONF_DIR} COMPONENT dev)
endif()

