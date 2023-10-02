###############################################################################
# CMake module to search for QGIS core library
#
# On success, the macro sets the following variables:
# QGIS_CORE_FOUND        = if the library found
# QGIS_CORE_LIBRARIES    = full path to the library
# QGIS_CORE_INCLUDE_DIRS = where to find the library headers
# QGIS_CORE_VERSION      = version as defined in qgsconfig.h, as major.minor.patch
#
# Copyright (c) 2020 NextGIS <info@nextgis.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
###############################################################################

IF(WIN32)

  SET(OSGEO4W_ROOT "C:/OSGeo4W")

  IF($ENV{OSGEO4W_HOME})
    SET(OSGEO4W_ROOT "$ENV{OSGEO4W_HOME}")
  ENDIF()

  IF($ENV{OSGEO4W_ROOT})
    SET(OSGEO4W_ROOT "$ENV{OSGEO4W_ROOT}")
  ENDIF()

  # OSGEO4W_QGIS_SUBDIR relative install: qgis[-rel|-ltr][-dev], etc.
  IF (NOT OSGEO4W_QGIS_SUBDIR OR "${OSGEO4W_QGIS_SUBDIR}" STREQUAL "")
    IF (NOT "$ENV{OSGEO4W_QGIS_SUBDIR}" STREQUAL "")
      SET (OSGEO4W_QGIS_SUBDIR $ENV{OSGEO4W_QGIS_SUBDIR})
    ELSE ()
      SET (OSGEO4W_QGIS_SUBDIR qgis)
    ENDIF ()
  ENDIF ()

  #MESSAGE("Searching for QGIS in $ENV{PROGRAMFILES}/QGIS")
  IF (MINGW)
    FIND_PATH(QGIS_CORE_INCLUDE_DIRS
      NAMES qgsapplication.h
      PATHS
        "$ENV{PROGRAMFILES}/QGIS/include"
    )
    FIND_LIBRARY(QGIS_CORE_LIBRARIES
      NAMES qgis_core
      PATHS
        "$ENV{PROGRAMFILES}/QGIS/"
    )
  ENDIF (MINGW)

  IF (MSVC)
    FIND_PATH(QGIS_CORE_INCLUDE_DIRS
      NAMES qgsapplication.h
      PATHS
        "$ENV{INCLUDE}"
        "$ENV{LIB_DIR}/include/qgis"
        "${OSGEO4W_ROOT}/include"
        "${OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/include"
        "$ENV{PROGRAMFILES}/QGIS/include"
    )
    FIND_LIBRARY(QGIS_CORE_LIBRARIES
      NAMES qgis_core
      PATHS
        "$ENV{LIB_DIR}/lib/"
        "$ENV{LIB}"
        "${OSGEO4W_ROOT}/lib"
        "${OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/lib"
        "$ENV{PROGRAMFILES}/QGIS/lib"
    )
  ENDIF (MSVC)
ELSE(WIN32)
  IF(UNIX)
    FIND_PATH(QGIS_CORE_INCLUDE_DIRS
      NAMES qgis.h
      PATHS
        ${QGIS_BUILD_PATH}/output/lib/qgis_core.framework/Headers
        ${QGIS_MAC_PATH}/Frameworks/qgis_core.framework/Headers
        {QGIS_PREFIX_PATH}/include/ngqgis
        {QGIS_PREFIX_PATH}/include/qgis
        /usr/include/ngqgis
        /usr/include/ngqgis/ngqgis
        /usr/local/include/ngqgis
        /usr/local/include/ngqgis/ngqgis
        /usr/include/qgis
        /usr/local/include/qgis
        /Library/Frameworks/qgis_core.framework/Headers
        "$ENV{LIB_DIR}/include/qgis"
    )
    FIND_LIBRARY(QGIS_CORE_LIBRARIES
      NAMES 
        ngqgis_core
        qgis_core
      PATHS
        ${QGIS_BUILD_PATH}/output/lib
        ${QGIS_MAC_PATH}/Frameworks
        ${QGIS_MAC_PATH}/lib
        ${QGIS_PREFIX_PATH}/lib/
        /usr/lib/x86_64-linux-gnu
        /usr/lib64
        /usr/lib32
        /usr/lib
        /usr/local/lib
        /Library/Frameworks
        "$ENV{LIB_DIR}/lib/"
    )
  ENDIF(UNIX)
ENDIF(WIN32)

message(STATUS "QGIS_CORE_INCLUDE_DIRS: ${QGIS_CORE_INCLUDE_DIRS} QGIS_CORE_LIBRARIES: ${QGIS_CORE_LIBRARIES}")

IF (QGIS_CORE_INCLUDE_DIRS)
  SET(QGIS_CORE_VERSION QGIS_VERSION-NOTFOUND)
  FIND_FILE(_qgsconfig_h qgsconfig.h PATHS ${QGIS_CORE_INCLUDE_DIRS})
  IF (_qgsconfig_h)
    FILE(READ ${_qgsconfig_h} _qgsconfig)
    IF (_qgsconfig)
      # version defined like #define VERSION "2.14.8-Essen"
      FILE(STRINGS "${_qgsconfig_h}" _qgsversion_str REGEX "^#define VERSION .*$")
      STRING(REGEX REPLACE "^#define VERSION +\"([0-9]+\\.[0-9]+\\.[0-9]+).*$" "\\1" _qgsversion "${_qgsversion_str}")
      IF (_qgsversion)
        SET(QGIS_CORE_VERSION ${_qgsversion})
      ELSE ()
        MESSAGE(WARNING "No QGIS core version determined: failed to parse qgsconfig.h")
      ENDIF ()
    ELSE()
      MESSAGE(WARNING "No QGIS core version determined: failed to read qgsconfig.h")
    ENDIF ()
  ELSE ()
    MESSAGE(WARNING "No QGIS core version determined: failed to find qgsconfig.h")
  ENDIF ()
ENDIF ()

IF (QGIS_CORE_INCLUDE_DIRS AND QGIS_CORE_LIBRARIES)
  SET(QGIS_CORE_FOUND TRUE)
ENDIF ()

IF (QGIS_CORE_FOUND)
  IF (QGIS_CORE_VERSION VERSION_LESS 3.22)
    MESSAGE(FATAL_ERROR "QGIS core minimum required version is 3.22 (found ${QGIS_CORE_VERSION})")
  ENDIF ()
  IF (NOT QGIS_CORE_FIND_QUIETLY)
    MESSAGE(STATUS "Found QGIS core: ${QGIS_CORE_LIBRARIES} (version ${QGIS_CORE_VERSION})")
  ENDIF (NOT QGIS_CORE_FIND_QUIETLY)
ELSE (QGIS_CORE_FOUND)
  MESSAGE(FATAL_ERROR "Could not find QGIS core")
ENDIF (QGIS_CORE_FOUND)
