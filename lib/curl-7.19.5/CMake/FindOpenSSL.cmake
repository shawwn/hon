# Extension of the standard FindOpenSSL.cmake
# Adds OPENSSL_INCLUDE_DIRS and libeay32
INCLUDE("${CMAKE_ROOT}/Modules/FindOpenSSL.cmake")

# Bill Hoffman told that libeay32 is necessary for him:
FIND_LIBRARY(SSL_LIBEAY NAMES libeay32)

IF(OPENSSL_FOUND)
	IF(SSL_LIBEAY)
		LIST(APPEND OPENSSL_LIBRARIES ${SSL_LIBEAY})
	ELSE()
		SET(OPENSSL_FOUND FALSE)
	ENDIF()
ENDIF()


IF(OPENSSL_FOUND)
	SET(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})
ENDIF()
