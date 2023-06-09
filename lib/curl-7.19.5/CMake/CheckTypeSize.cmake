# - Check sizeof a type
#  CHECK_TYPE_SIZE(TYPE VARIABLE)
# Check if the type exists and determine size of type.  if the type
# exists, the size will be stored to the variable.
#
#  VARIABLE - variable to store size if the type exists.
#  HAVE_${VARIABLE} - does the variable exists or not

MACRO(CHECK_TYPE_SIZE TYPE VARIABLE)
  SET(CMAKE_ALLOW_UNKNOWN_VARIABLE_READ_ACCESS 1)
  IF(NOT DEFINED ${VARIABLE})
  IF("HAVE_${VARIABLE}" MATCHES "^HAVE_${VARIABLE}$")
    SET(CHECK_TYPE_SIZE_TYPE "${TYPE}")
    SET(MACRO_CHECK_TYPE_SIZE_FLAGS 
      "${CMAKE_REQUIRED_FLAGS}")
    FOREACH(def HAVE_SYS_TYPES_H HAVE_STDINT_H HAVE_STDDEF_H)
      IF("${def}")
        SET(MACRO_CHECK_TYPE_SIZE_FLAGS 
          "${MACRO_CHECK_TYPE_SIZE_FLAGS} -D${def}")
      ENDIF("${def}")
    ENDFOREACH(def)
    SET(CHECK_TYPE_SIZE_PREMAIN)
    FOREACH(def ${CMAKE_EXTRA_INCLUDE_FILES})
      SET(CHECK_TYPE_SIZE_PREMAIN "${CHECK_TYPE_SIZE_PREMAIN}#include \"${def}\"\n")
    ENDFOREACH(def)
    CONFIGURE_FILE(
      "${CMAKE_CURRENT_SOURCE_DIR}/CMake/CheckTypeSize.c.in"
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckTypeSize.c" 
      IMMEDIATE @ONLY)
    FILE(READ 
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckTypeSize.c"
      CHECK_TYPE_SIZE_FILE_CONTENT)
    MESSAGE(STATUS "Check size of ${TYPE}")
    IF(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_TYPE_SIZE_ADD_LIBRARIES 
        "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    ENDIF(CMAKE_REQUIRED_LIBRARIES)
    TRY_RUN(${VARIABLE} HAVE_${VARIABLE}
      ${CMAKE_BINARY_DIR}
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckTypeSize.c"
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_TYPE_SIZE_FLAGS}
      "${CHECK_TYPE_SIZE_ADD_LIBRARIES}"
      OUTPUT_VARIABLE OUTPUT)
    IF(HAVE_${VARIABLE})
      MESSAGE(STATUS "Check size of ${TYPE} - done")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log 
        "Determining size of ${TYPE} passed with the following output:\n${OUTPUT}\n\n")
    ELSE(HAVE_${VARIABLE})
      MESSAGE(STATUS "Check size of ${TYPE} - failed")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
        "Determining size of ${TYPE} failed with the following output:\n${OUTPUT}\nCheckTypeSize.c:\n${CHECK_TYPE_SIZE_FILE_CONTENT}\n\n")
    ENDIF(HAVE_${VARIABLE})
  ENDIF("HAVE_${VARIABLE}" MATCHES "^HAVE_${VARIABLE}$")
  ENDIF(NOT DEFINED ${VARIABLE})
  SET(CMAKE_ALLOW_UNKNOWN_VARIABLE_READ_ACCESS )
ENDMACRO(CHECK_TYPE_SIZE)
