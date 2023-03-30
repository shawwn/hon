# File containing various utilities

# Converts a CMake list to a string containing elements separated by spaces
FUNCTION(TO_LIST_SPACES _LIST_NAME OUTPUT_VAR)
  SET(NEW_LIST_SPACE)
  FOREACH(ITEM ${${_LIST_NAME}})
    SET(NEW_LIST_SPACE "${NEW_LIST_SPACE} ${ITEM}")
  ENDFOREACH()
  STRING(STRIP ${NEW_LIST_SPACE} NEW_LIST_SPACE)
  SET(${OUTPUT_VAR} "${NEW_LIST_SPACE}" PARENT_SCOPE)
ENDFUNCTION()

# Appends a lis of item to a string which is a space-separated list, if they don't already exist.
FUNCTION(LIST_SPACES_APPEND_ONCE LIST_NAME)
  STRING(REPLACE " " ";" _LIST ${${LIST_NAME}})
  LIST(APPEND _LIST ${ARGN})
  LIST(REMOVE_DUPLICATES _LIST)
  TO_LIST_SPACES(_LIST NEW_LIST_SPACE)
  SET(${LIST_NAME} "${NEW_LIST_SPACE}" PARENT_SCOPE)
ENDFUNCTION()

# Convinience function that does the same as LIST(FIND ...) but with a TRUE/FALSE return value.
# Ex: IN_STR_LIST(MY_LIST "Searched item" WAS_FOUND)
FUNCTION(IN_STR_LIST LIST_NAME ITEM_SEARCHED RETVAL)
	LIST(FIND ${LIST_NAME} ${ITEM_SEARCHED} FIND_POS)
	IF(${FIND_POS} EQUAL -1)
		SET(${RETVAL} FALSE PARENT_SCOPE)
	ELSE()
		SET(${RETVAL} TRUE PARENT_SCOPE)
	ENDIF()
ENDFUNCTION()
