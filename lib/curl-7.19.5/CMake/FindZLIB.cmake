# Locate zlib
INCLUDE("${CMAKE_ROOT}/Modules/FindZLIB.cmake")

FIND_LIBRARY(ZLIB_LIBRARY_DEBUG NAMES zd zlibd zdlld zlib1d )

IF(ZLIB_FOUND AND ZLIB_LIBRARY_DEBUG)
  SET( ZLIB_LIBRARIES optimized "${ZLIB_LIBRARY}" debug ${ZLIB_LIBRARY_DEBUG})
ENDIF()
