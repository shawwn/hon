# MojoSetup; a portable, flexible installation application.
#
# Please see the file LICENSE.txt in the source's root directory.
#
#  This file written by Ryan C. Gordon.



# The "BINARY SIZE +=" comments note about how much bulk, in kilobytes, a
#  given option adds to the binary on x86 Linux (built with gcc 3.3.6
#  MinSizeRel options and stripped, uncompressed). These numbers will vary,
#  and even on the original test system, become incorrect over time.
# Only choose options you want/need to squeeze every byte off the download.

PROJECT(MojoSetup)

EXECUTE_PROCESS(
    COMMAND "svnversion"
    WORKING_DIRECTORY "."
    RESULT_VARIABLE SVNVERSION_RC
    OUTPUT_VARIABLE MOJOSETUP_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

IF(SVNVERSION_RC)
    SET(MOJOSETUP_VERSION "???")
ENDIF(SVNVERSION_RC)

SET(MOJOSETUP_VERSION "svn-${MOJOSETUP_VERSION}")


# I hate that they define "WIN32" ... we're about to move to Win64...I hope!
IF(WIN32 AND NOT WINDOWS)
    SET(WINDOWS TRUE)
ENDIF(WIN32 AND NOT WINDOWS)

# Bleh, let's do it for "APPLE" too.
IF(APPLE AND NOT MACOSX)
    SET(MACOSX TRUE)
ENDIF(APPLE AND NOT MACOSX)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckCCompilerFlag)
INCLUDE(TestBigEndian)

ADD_DEFINITIONS(-D__MOJOSETUP__=1)
ADD_DEFINITIONS(-DAPPID=mojosetup)
ADD_DEFINITIONS(-DAPPREV="${MOJOSETUP_VERSION}")
ADD_DEFINITIONS(-D_REENTRANT)
ADD_DEFINITIONS(-D_THREAD_SAFE)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(lua-5.1.1/src)

IF(WINDOWS)
    ADD_DEFINITIONS(-DPLATFORM_WINDOWS=1)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS=1)
    SET(USES_WINMAIN WIN32)
    SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} shell32)
ENDIF(WINDOWS)

IF(MACOSX)
    ADD_DEFINITIONS(-DPLATFORM_MACOSX=1)
    ADD_DEFINITIONS(-DLUA_USE_DYLD=1)
    IF(CMAKE_OSX_ARCHITECTURES MATCHES ppc)
        ADD_DEFINITIONS(-DMAC_OS_X_VERSION_MIN_REQUIRED=1020)
        SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} "-mmacosx-version-min=10.2")
    ENDIF(CMAKE_OSX_ARCHITECTURES MATCHES ppc)
    SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} "-framework Carbon")
ENDIF(MACOSX)

IF(BEOS)
    ADD_DEFINITIONS(-DPLATFORM_BEOS=1)
    # !!! FIXME: Workaround for lua issue...fix this better.
    ADD_DEFINITIONS(-D_setjmp=setjmp)
    ADD_DEFINITIONS(-D_longjmp=longjmp)
ENDIF(BEOS)

IF(UNIX)
    ADD_DEFINITIONS(-DPLATFORM_UNIX=1)
    ADD_DEFINITIONS(-DLUA_USE_POSIX=1)
    IF(NOT MACOSX AND NOT BEOS)
        ADD_DEFINITIONS(-DLUA_USE_DLOPEN=1)
    ENDIF(NOT MACOSX AND NOT BEOS)
ENDIF(UNIX)

IF(CMAKE_COMPILER_IS_GNUCC)
    ADD_DEFINITIONS(-pipe -Wall -Werror -fsigned-char)

    # See if -fvisibility=hidden is available to us.
    CHECK_C_SOURCE_COMPILES("
        #if ((defined(__GNUC__)) && (__GNUC__ >= 4))
        int main(int argc, char **argv) { int is_gcc4 = 1; return 0; }
        #else
        #error This is not gcc4.
        #endif
    " MOJOSETUP_IS_GCC4)

    IF(MOJOSETUP_IS_GCC4)
        ADD_DEFINITIONS(-fvisibility=hidden)
    ENDIF(MOJOSETUP_IS_GCC4)

    # See if -fno-stack-protector is available to us.
    # It doesn't seem to work well, and it adds bulk to the binary.
    CHECK_C_COMPILER_FLAG("-fno-stack-protector" MOJOSETUP_GCC_HAS_STACKPROT)
    IF(MOJOSETUP_GCC_HAS_STACKPROT)
        ADD_DEFINITIONS(-fno-stack-protector)
    ENDIF(MOJOSETUP_GCC_HAS_STACKPROT)

    # !!! FIXME: probably not safe long-term.
    # CMake mailing list had this hack for getting rid of -rdynamic:
    #   http://public.kitware.com/pipermail/cmake/2006-July/010404.html
    IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
        SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
    ENDIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    # Don't use -rpath.
    SET(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

TEST_BIG_ENDIAN(MOJOSETUP_IS_BIGENDIAN)
IF(MOJOSETUP_IS_BIGENDIAN)
    ADD_DEFINITIONS(-DPLATFORM_BIGENDIAN=1)
ELSE(MOJOSETUP_IS_BIGENDIAN)
    ADD_DEFINITIONS(-DPLATFORM_LITTLEENDIAN=1)
ENDIF(MOJOSETUP_IS_BIGENDIAN)

SET(ZLIB_DIR zlib-1.2.3)
SET(ZLIB_SRCS
    ${ZLIB_DIR}/adler32.c
    ${ZLIB_DIR}/compress.c
    ${ZLIB_DIR}/crc32.c
    ${ZLIB_DIR}/deflate.c
    ${ZLIB_DIR}/gzio.c
    ${ZLIB_DIR}/infback.c
    ${ZLIB_DIR}/inffast.c
    ${ZLIB_DIR}/inflate.c
    ${ZLIB_DIR}/inftrees.c
    ${ZLIB_DIR}/trees.c
    ${ZLIB_DIR}/uncompr.c
    ${ZLIB_DIR}/zutil.c
)

SET(BZLIB_DIR bzip2-1.0.4)
SET(BZLIB_SRCS
    ${BZLIB_DIR}/blocksort.c
    ${BZLIB_DIR}/huffman.c
    ${BZLIB_DIR}/crctable.c
    ${BZLIB_DIR}/randtable.c
    ${BZLIB_DIR}/compress.c
    ${BZLIB_DIR}/decompress.c
    ${BZLIB_DIR}/bzlib.c
)

SET(LIBFETCH_DIR libfetch)
SET(LIBFETCH_SRCS
    ${LIBFETCH_DIR}/fetch.c
    ${LIBFETCH_DIR}/common.c
    ${LIBFETCH_DIR}/ftp.c
    ${LIBFETCH_DIR}/http.c
)

SET(LUA_DIR lua-5.1.1)
SET(LUA_SRCS
    ${LUA_DIR}/src/lapi.c
    ${LUA_DIR}/src/ldebug.c
    ${LUA_DIR}/src/ldo.c
    ${LUA_DIR}/src/ldump.c
    ${LUA_DIR}/src/lfunc.c
    ${LUA_DIR}/src/lgc.c
    ${LUA_DIR}/src/lmem.c
    ${LUA_DIR}/src/lobject.c
    ${LUA_DIR}/src/lopcodes.c
    ${LUA_DIR}/src/lstate.c
    ${LUA_DIR}/src/lstring.c
    ${LUA_DIR}/src/ltable.c
    ${LUA_DIR}/src/ltm.c
    ${LUA_DIR}/src/lundump.c
    ${LUA_DIR}/src/lvm.c
    ${LUA_DIR}/src/lzio.c
    ${LUA_DIR}/src/lauxlib.c
    ${LUA_DIR}/src/lbaselib.c
    ${LUA_DIR}/src/lstrlib.c
    ${LUA_DIR}/src/ltablib.c
)

SET(LUA_PARSER_SRCS
    ${LUA_DIR}/src/lparser.c
    ${LUA_DIR}/src/llex.c
    ${LUA_DIR}/src/lcode.c
)

SET(MOJOSETUP_SRCS
    buildver.c
    mojosetup.c
    gui.c
    fileio.c
    archive_zip.c
    archive_tar.c
    checksum_crc32.c
    checksum_md5.c
    checksum_sha1.c
    platform_unix.c
    platform_windows.c
    lua_glue.c
    ${LUA_SRCS}
)

# Have to separate this, so CMake doesn't try to link in C++ support on other
#  platforms.
IF(BEOS)
    SET(MOJOSETUP_SRCS ${MOJOSETUP_SRCS} platform_beos.cpp)
ENDIF(BEOS)

SET(MOJOLUAC_SRCS
    ${LUA_SRCS}
    ${LUA_PARSER_SRCS}
    ${LUA_DIR}/src/luac.c
    ${LUA_DIR}/src/print.c
    ${LUA_DIR}/src/linit.c
    ${LUA_DIR}/src/ldblib.c
    ${LUA_DIR}/src/liolib.c
    ${LUA_DIR}/src/lmathlib.c
    ${LUA_DIR}/src/loslib.c
    ${LUA_DIR}/src/loadlib.c
)

SET(STBIMAGE_SRCS
    stb_image.c
)

# Disabling the parser cuts the Lua binary bits by about 35%, plus .luac files
#  are almost always smaller than the original scripts. The downside is you
#  (and end users in the field) can't just tweak a script without recompiling
#  it, but even that's not an unclimbable obstacle.
# In reality, you probably want to keep the parser, though, unless you REALLY
#  must save every single byte in the download.
# YOU NEED THE PARSER IF YOU WANT MANIFESTS WRITTEN OUT.
# YOU NEED THE PARSER IF YOU WANT THE UNINSTALLER TO WORK.
# DON'T DISABLE THIS NOW IF YOU DON'T ABSOLUTELY HAVE TO.
# BINARY SIZE += 19
OPTION(MOJOSETUP_LUA_PARSER "Bigger binary but scripts don't need to be compiled." TRUE)
IF(MOJOSETUP_LUA_PARSER)
    SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${LUA_PARSER_SRCS})
ELSE(MOJOSETUP_LUA_PARSER)
    SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${LUA_DIR}/etc/noparser.c)
    ADD_DEFINITIONS(-DDISABLE_LUA_PARSER=1)
ENDIF(MOJOSETUP_LUA_PARSER)


# Kludge for Linux x86/amd64 bins...
IF(UNIX AND NOT MACOSX)  # Just use Mach-O Universal/"fat" binaries on OS X.
    OPTION(MOJOSETUP_MULTIARCH "Allow multiarch hack." FALSE)
    MARK_AS_ADVANCED(MOJOSETUP_MULTIARCH)
    IF(MOJOSETUP_MULTIARCH)
        ADD_DEFINITIONS(-DSUPPORT_MULTIARCH=1)
    ENDIF(MOJOSETUP_MULTIARCH)
ENDIF(UNIX AND NOT MACOSX)


# Optional bits of the Lua runtime library...

# BINARY SIZE += 4.5
OPTION(MOJOSETUP_LUALIB_IO "Add Lua 'io' library" TRUE)
MARK_AS_ADVANCED(MOJOSETUP_LUALIB_IO)
IF(MOJOSETUP_LUALIB_IO)
    ADD_DEFINITIONS(-DSUPPORT_LUALIB_IO=1)
    SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${LUA_DIR}/src/liolib.c)
ENDIF(MOJOSETUP_LUALIB_IO)

# BINARY SIZE += 2.5
OPTION(MOJOSETUP_LUALIB_OS "Add Lua 'os' library" TRUE)
MARK_AS_ADVANCED(MOJOSETUP_LUALIB_OS)
IF(MOJOSETUP_LUALIB_OS)
    ADD_DEFINITIONS(-DSUPPORT_LUALIB_OS=1)
    SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${LUA_DIR}/src/loslib.c)
ENDIF(MOJOSETUP_LUALIB_OS)

# BINARY SIZE += 3.5
OPTION(MOJOSETUP_LUALIB_MATH "Add Lua 'math' library" TRUE)
MARK_AS_ADVANCED(MOJOSETUP_LUALIB_MATH)
IF(MOJOSETUP_LUALIB_MATH)
    ADD_DEFINITIONS(-DSUPPORT_LUALIB_MATH=1)
    SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${LUA_DIR}/src/lmathlib.c)
ENDIF(MOJOSETUP_LUALIB_MATH)

# BINARY SIZE += 3.5
OPTION(MOJOSETUP_LUALIB_DB "Add Lua 'db' library" TRUE)
MARK_AS_ADVANCED(MOJOSETUP_LUALIB_DB)
IF(MOJOSETUP_LUALIB_DB)
    ADD_DEFINITIONS(-DSUPPORT_LUALIB_DB=1)
    SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${LUA_DIR}/src/ldblib.c)
ENDIF(MOJOSETUP_LUALIB_DB)

# BINARY SIZE += 4
OPTION(MOJOSETUP_LUALIB_PACKAGE "Add Lua 'package' library" TRUE)
MARK_AS_ADVANCED(MOJOSETUP_LUALIB_PACKAGE)
IF(MOJOSETUP_LUALIB_PACKAGE)
    ADD_DEFINITIONS(-DSUPPORT_LUALIB_PACKAGE=1)
    SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${LUA_DIR}/src/loadlib.c)
ENDIF(MOJOSETUP_LUALIB_PACKAGE)


# Checksums...

# BINARY SIZE += !!! FIXME: check this.
OPTION(MOJOSETUP_CHECKSUM_CRC32 "Enable CRC-32 checksum support" TRUE)
IF(MOJOSETUP_CHECKSUM_CRC32)
    ADD_DEFINITIONS(-DSUPPORT_CRC32=1)
ENDIF(MOJOSETUP_CHECKSUM_CRC32)

# BINARY SIZE += !!! FIXME: check this.
OPTION(MOJOSETUP_CHECKSUM_MD5 "Enable MD5 checksum support" TRUE)
IF(MOJOSETUP_CHECKSUM_MD5)
    ADD_DEFINITIONS(-DSUPPORT_MD5=1)
ENDIF(MOJOSETUP_CHECKSUM_MD5)

# BINARY SIZE += !!! FIXME: check this.
OPTION(MOJOSETUP_CHECKSUM_SHA1 "Enable SHA-1 checksum support" TRUE)
IF(MOJOSETUP_CHECKSUM_SHA1)
    ADD_DEFINITIONS(-DSUPPORT_SHA1=1)
ENDIF(MOJOSETUP_CHECKSUM_SHA1)


# GUI plugins...

MACRO(MOJOSETUP_ADD_LIBRARY _TARGET _SRCS)
    ADD_LIBRARY(${_TARGET} SHARED ${_SRCS})
    SET(MOJOSETUP_TARGETS "${MOJOSETUP_TARGETS};${_TARGET}")
ENDMACRO(MOJOSETUP_ADD_LIBRARY)


# BINARY SIZE += 2.5
OPTION(MOJOSETUP_GUI_STDIO "Enable stdio GUI" TRUE)
IF(MOJOSETUP_GUI_STDIO)
    ADD_DEFINITIONS(-DSUPPORT_GUI_STDIO=1)
    OPTION(MOJOSETUP_GUI_STDIO_STATIC "Statically link stdio GUI" TRUE)
    IF(MOJOSETUP_GUI_STDIO_STATIC)
        ADD_DEFINITIONS(-DGUI_STATIC_LINK_STDIO=1)
        SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} gui_stdio.c)
    ELSE(MOJOSETUP_GUI_STDIO_STATIC)
        MOJOSETUP_ADD_LIBRARY(mojosetupgui_stdio gui_stdio.c)
    ENDIF(MOJOSETUP_GUI_STDIO_STATIC)
ENDIF(MOJOSETUP_GUI_STDIO)

# BINARY SIZE += !!! FIXME: check this.
IF(UNIX)  # !!! FIXME: use FindCurses instead...
IF(NOT BEOS)
IF(NOT MACOSX)
OPTION(MOJOSETUP_GUI_NCURSES "Enable ncurses GUI" TRUE)
IF(MOJOSETUP_GUI_NCURSES)
    ADD_DEFINITIONS(-DSUPPORT_GUI_NCURSES=1)
    OPTION(MOJOSETUP_GUI_NCURSES_STATIC "Statically link ncurses GUI" FALSE)
    IF(MOJOSETUP_GUI_NCURSES_STATIC)
        ADD_DEFINITIONS(-DGUI_STATIC_LINK_NCURSES=1)
        SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} gui_ncurses.c)
        SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} -lncurses)  # !!! FIXME
    ELSE(MOJOSETUP_GUI_NCURSES_STATIC)
        MOJOSETUP_ADD_LIBRARY(mojosetupgui_ncurses gui_ncurses.c)
        TARGET_LINK_LIBRARIES(mojosetupgui_ncurses "-lncurses")  # !!! FIXME
    ENDIF(MOJOSETUP_GUI_NCURSES_STATIC)
ENDIF(MOJOSETUP_GUI_NCURSES)
ENDIF(NOT MACOSX)
ENDIF(NOT BEOS)
ENDIF(UNIX)

IF(MACOSX)
    OPTION(MOJOSETUP_GUI_MACOSX "Enable Mac OS X GUI" TRUE)
    IF(MOJOSETUP_GUI_MACOSX)
        ADD_DEFINITIONS(-DSUPPORT_GUI_MACOSX=1)
        OPTION(MOJOSETUP_GUI_MACOSX_STATIC "Statically link Mac OS X GUI" TRUE)
        IF(MOJOSETUP_GUI_MACOSX_STATIC)
            ADD_DEFINITIONS(-DGUI_STATIC_LINK_MACOSX=1)
            SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} gui_macosx.c)
        ELSE(MOJOSETUP_GUI_MACOSX_STATIC)
            MOJOSETUP_ADD_LIBRARY(mojosetupgui_macosx gui_macosx.c)
            TARGET_LINK_LIBRARIES(mojosetupgui_macosx
                "-framework Carbon -mmacosx-version-min=10.2"
            )
        ENDIF(MOJOSETUP_GUI_MACOSX_STATIC)
    ENDIF(MOJOSETUP_GUI_MACOSX)
ENDIF(MACOSX)

IF(UNIX)
IF(NOT BEOS)
IF(NOT MACOSX)
#FIND_PACKAGE(PkgConfig)
#PKGCONFIG(libgtk-2.0 LIBGTK_INCLUDE_DIR LIBGTK_LINK_DIR LIBGTK_LINK_FLAGS LIBGTK_CFLAGS)
# !!! FIXME
SET(LIBGTK_LINK_FLAGS "-lgtk-x11-2.0")
SET(LIBGTK_CFLAGS "-DPNG_NO_MMX_CODE -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/libpng12")
IF(NOT LIBGTK_LINK_FLAGS)
    MESSAGE(STATUS "Can't find GTK+v2 headers/libraries. Can't build GTK+ GUI.")
ELSE(NOT LIBGTK_LINK_FLAGS)
    OPTION(MOJOSETUP_GUI_GTKPLUS2 "Enable GTK+ 2.0 GUI" TRUE)
    IF(MOJOSETUP_GUI_GTKPLUS2)
        ADD_DEFINITIONS(-DSUPPORT_GUI_GTKPLUS2=1)
        #INCLUDE_DIRECTORIES(${LIBGTK_INCLUDE_DIR})
        OPTION(MOJOSETUP_GUI_GTKPLUS2_STATIC "Statically link GTK+ GUI" FALSE)
        IF(MOJOSETUP_GUI_GTKPLUS2_STATIC)
            ADD_DEFINITIONS(-DGUI_STATIC_LINK_GTKPLUS2=1 ${LIBGTK_CFLAGS})
            SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} gui_gtkplus2.c)
            SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} ${LIBGTK_LINK_FLAGS})
        ELSE(MOJOSETUP_GUI_GTKPLUS2_STATIC)
            MOJOSETUP_ADD_LIBRARY(mojosetupgui_gtkplus2 gui_gtkplus2.c)
            ADD_DEFINITIONS(${LIBGTK_CFLAGS})
            TARGET_LINK_LIBRARIES(mojosetupgui_gtkplus2 ${LIBGTK_LINK_FLAGS})
        ENDIF(MOJOSETUP_GUI_GTKPLUS2_STATIC)
    ENDIF(MOJOSETUP_GUI_GTKPLUS2)
ENDIF(NOT LIBGTK_LINK_FLAGS)
ENDIF(NOT MACOSX)
ENDIF(NOT BEOS)
ENDIF(UNIX)

# BINARY SIZE += !!! FIXME: check this.
OPTION(MOJOSETUP_GUI_WWW "Enable www GUI" FALSE)  # !!! FIXME: make TRUE
IF(MOJOSETUP_GUI_WWW)
    ADD_DEFINITIONS(-DSUPPORT_GUI_WWW=1)
    OPTION(MOJOSETUP_GUI_WWW_STATIC "Statically link www GUI" FALSE)
    IF(MOJOSETUP_GUI_WWW_STATIC)
        ADD_DEFINITIONS(-DGUI_STATIC_LINK_WWW=1)
        SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} gui_www.c)
        IF(WINDOWS)
            SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} wsock32)
        ENDIF(WINDOWS)
    ELSE(MOJOSETUP_GUI_WWW_STATIC)
        MOJOSETUP_ADD_LIBRARY(mojosetupgui_www gui_www.c)
        IF(WINDOWS)
            TARGET_LINK_LIBRARIES(mojosetupgui_www wsock32)
        ENDIF(WINDOWS)
    ENDIF(MOJOSETUP_GUI_WWW_STATIC)
ENDIF(MOJOSETUP_GUI_WWW)


# Archivers...

# BINARY SIZE += 8
OPTION(MOJOSETUP_ARCHIVE_ZIP "Enable ZIP support" TRUE)
IF(MOJOSETUP_ARCHIVE_ZIP)
    ADD_DEFINITIONS(-DSUPPORT_ZIP=1)
    SET(MOJOSETUP_NEED_ZLIB TRUE)
ENDIF(MOJOSETUP_ARCHIVE_ZIP)

# BINARY SIZE += 2
OPTION(MOJOSETUP_ARCHIVE_TAR "Enable TAR support" TRUE)
IF(MOJOSETUP_ARCHIVE_TAR)
    ADD_DEFINITIONS(-DSUPPORT_TAR=1)

    # BINARY SIZE += 1.5
    OPTION(MOJOSETUP_ARCHIVE_TAR_GZ "Enable TAR.GZ support" TRUE)
    IF(MOJOSETUP_ARCHIVE_TAR_GZ)
        ADD_DEFINITIONS(-DSUPPORT_TAR_GZ=1)
        SET(MOJOSETUP_NEED_ZLIB TRUE)
    ENDIF(MOJOSETUP_ARCHIVE_TAR_GZ)

    # BINARY SIZE += 1.5
    OPTION(MOJOSETUP_ARCHIVE_TAR_BZ2 "Enable TAR.BZ2 support" TRUE)
    IF(MOJOSETUP_ARCHIVE_TAR_BZ2)
        ADD_DEFINITIONS(-DSUPPORT_TAR_BZ2=1)
        ADD_DEFINITIONS(-DBZ_NO_STDIO=1)
        SET(MOJOSETUP_NEED_BZLIB TRUE)
    ENDIF(MOJOSETUP_ARCHIVE_TAR_BZ2)
ENDIF(MOJOSETUP_ARCHIVE_TAR)


# Image decoders for GUIs...

OPTION(MOJOSETUP_IMAGE_JPG "Enable JPG support" TRUE)
IF(MOJOSETUP_IMAGE_JPG)
    ADD_DEFINITIONS(-DSUPPORT_JPG=1)
    SET(MOJOSETUP_NEED_STBIMAGE TRUE)
ENDIF(MOJOSETUP_IMAGE_JPG)

OPTION(MOJOSETUP_IMAGE_PNG "Enable PNG support" TRUE)
IF(MOJOSETUP_IMAGE_PNG)
    ADD_DEFINITIONS(-DSUPPORT_PNG=1)
    SET(MOJOSETUP_NEED_STBIMAGE TRUE)
ENDIF(MOJOSETUP_IMAGE_PNG)

OPTION(MOJOSETUP_IMAGE_TGA "Enable TGA support" FALSE)
IF(MOJOSETUP_IMAGE_TGA)
    ADD_DEFINITIONS(-DSUPPORT_TGA=1)
    SET(MOJOSETUP_NEED_STBIMAGE TRUE)
ENDIF(MOJOSETUP_IMAGE_TGA)

OPTION(MOJOSETUP_IMAGE_BMP "Enable BMP support" FALSE)
IF(MOJOSETUP_IMAGE_BMP)
    ADD_DEFINITIONS(-DSUPPORT_BMP=1)
    SET(MOJOSETUP_NEED_STBIMAGE TRUE)
ENDIF(MOJOSETUP_IMAGE_BMP)

OPTION(MOJOSETUP_IMAGE_PSD "Enable PSD support" FALSE)
IF(MOJOSETUP_IMAGE_PSD)
    ADD_DEFINITIONS(-DSUPPORT_PSD=1)
    SET(MOJOSETUP_NEED_STBIMAGE TRUE)
ENDIF(MOJOSETUP_IMAGE_PSD)

OPTION(MOJOSETUP_IMAGE_HDR "Enable HDR support" FALSE)
IF(MOJOSETUP_IMAGE_HDR)
    ADD_DEFINITIONS(-DSUPPORT_HDR=1)
    SET(MOJOSETUP_NEED_STBIMAGE TRUE)
ENDIF(MOJOSETUP_IMAGE_HDR)

IF(MOJOSETUP_NEED_STBIMAGE)
    ADD_DEFINITIONS(-DSUPPORT_STBIMAGE=1)
    SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${STBIMAGE_SRCS})
ENDIF(MOJOSETUP_NEED_STBIMAGE)


# Networking...

# BINARY SIZE += 5  ...plus libfetch
OPTION(MOJOSETUP_URL_HTTP "Enable http:// support" TRUE)
IF(MOJOSETUP_URL_HTTP)
    ADD_DEFINITIONS(-DSUPPORT_URL_HTTP=1)
    SET(MOJOSETUP_NEED_LIBFETCH TRUE)
ENDIF(MOJOSETUP_URL_HTTP)

# BINARY SIZE += 9  ...plus libfetch
OPTION(MOJOSETUP_URL_FTP "Enable ftp://  support" TRUE)
IF(MOJOSETUP_URL_FTP)
    ADD_DEFINITIONS(-DSUPPORT_URL_FTP=1)
    SET(MOJOSETUP_NEED_LIBFETCH TRUE)
ENDIF(MOJOSETUP_URL_FTP)

# BINARY SIZE += 10
IF(MOJOSETUP_NEED_LIBFETCH)
    SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${LIBFETCH_SRCS})
    # Had to spin up some threads in libfetch...lame.
    # !!! FIXME: CMake will do -lpthread on Mac OS X, but it doesn't need it.
    IF(NOT MACOSX)
        FIND_PACKAGE(Threads)
    ENDIF(NOT MACOSX)
    SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(MOJOSETUP_NEED_LIBFETCH)


# Middleware...

IF(MOJOSETUP_NEED_ZLIB)
    SET(HAVE_SYSTEM_ZLIB FALSE)
    CHECK_INCLUDE_FILE(zlib.h HAVE_ZLIB_H)
    IF(HAVE_ZLIB_H)
        CHECK_LIBRARY_EXISTS("z" "inflate" "" HAVE_LIBZ)
        IF(HAVE_LIBZ)
            SET(HAVE_SYSTEM_ZLIB TRUE)
        ENDIF(HAVE_LIBZ)
    ENDIF(HAVE_ZLIB_H)

    IF(HAVE_SYSTEM_ZLIB)
        OPTION(MOJOSETUP_INTERNAL_ZLIB "Link own zlib instead of system library" FALSE)
    ELSE(HAVE_SYSTEM_ZLIB)
        SET(MOJOSETUP_INTERNAL_ZLIB TRUE)
    ENDIF(HAVE_SYSTEM_ZLIB)

    # BINARY SIZE += 50
    IF(MOJOSETUP_INTERNAL_ZLIB)
        INCLUDE_DIRECTORIES(${ZLIB_DIR})
        ADD_DEFINITIONS(-DZ_PREFIX=1)
        SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${ZLIB_SRCS})
    ELSE(MOJOSETUP_INTERNAL_ZLIB)
        SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} z)
    ENDIF(MOJOSETUP_INTERNAL_ZLIB)
ENDIF(MOJOSETUP_NEED_ZLIB)

IF(MOJOSETUP_NEED_BZLIB)
    SET(HAVE_SYSTEM_BZLIB FALSE)
    CHECK_INCLUDE_FILE(bzlib.h HAVE_BZLIB_H)
    IF(HAVE_BZLIB_H)
        CHECK_LIBRARY_EXISTS("bz2" "BZ2_bzDecompress" "" HAVE_LIBBZ2)
        IF(HAVE_LIBBZ2)
            SET(HAVE_SYSTEM_BZLIB TRUE)
        ENDIF(HAVE_LIBBZ2)
    ENDIF(HAVE_BZLIB_H)

    IF(HAVE_SYSTEM_BZLIB)
        OPTION(MOJOSETUP_INTERNAL_BZLIB "Link own bzlib instead of system library" FALSE)
    ELSE(HAVE_SYSTEM_BZLIB)
        SET(MOJOSETUP_INTERNAL_BZLIB TRUE)
    ENDIF(HAVE_SYSTEM_BZLIB)

    # BINARY SIZE += 46
    IF(MOJOSETUP_INTERNAL_BZLIB)
        ADD_DEFINITIONS(-DMOJOSETUP_INTERNAL_BZLIB=1)
        INCLUDE_DIRECTORIES(${BZLIB_DIR})
        SET(OPTIONAL_SRCS ${OPTIONAL_SRCS} ${BZLIB_SRCS})
    ELSE(MOJOSETUP_INTERNAL_BZLIB)
        SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} bz2)
    ENDIF(MOJOSETUP_INTERNAL_BZLIB)
ENDIF(MOJOSETUP_NEED_BZLIB)

IF(UNIX)
    CHECK_INCLUDE_FILE(sys/ucred.h HAVE_UCRED_H)
    IF(HAVE_UCRED_H)
        ADD_DEFINITIONS(-DMOJOSETUP_HAVE_SYS_UCRED_H=1)
    ENDIF(HAVE_UCRED_H)

    CHECK_INCLUDE_FILE(mntent.h HAVE_MNTENT_H)
    IF(HAVE_MNTENT_H)
        ADD_DEFINITIONS(-DMOJOSETUP_HAVE_MNTENT_H=1)
    ENDIF(HAVE_MNTENT_H)

    IF(NOT MACOSX)
        CHECK_LIBRARY_EXISTS("dl" "dlopen" "" HAVE_LIBDL)
        IF(HAVE_LIBDL)
            SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} dl)
        ENDIF(HAVE_LIBDL)
        CHECK_LIBRARY_EXISTS("m" "sin" "" HAVE_LIBM)
        IF(HAVE_LIBM)
            SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} m)
        ENDIF(HAVE_LIBM)
    ENDIF(NOT MACOSX)
ENDIF(UNIX)

OPTION(MOJOSETUP_BUILD_LUAC "Build separate Lua compiler" TRUE)
IF(MOJOSETUP_BUILD_LUAC)
    ADD_EXECUTABLE(mojoluac ${MOJOLUAC_SRCS})
    TARGET_LINK_LIBRARIES(mojoluac ${OPTIONAL_LIBS})
    GET_TARGET_PROPERTY(MOJOLUAC_LOCATION mojoluac LOCATION)
    # !!! FIXME: actually compile this.
    ADD_CUSTOM_TARGET(lua "${MOJOLUAC_LOCATION}" -p ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.lua)
ENDIF(MOJOSETUP_BUILD_LUAC)

ADD_EXECUTABLE(mojosetup ${USES_WINMAIN} ${MOJOSETUP_SRCS} ${OPTIONAL_SRCS})
GET_TARGET_PROPERTY(MOJOSETUP_BINARY_LOCATION mojosetup LOCATION)
TARGET_LINK_LIBRARIES(mojosetup ${OPTIONAL_LIBS})
SET(MOJOSETUP_TARGETS "${MOJOSETUP_TARGETS};mojosetup")

# For cobbling together a skeleton installer...

ADD_CUSTOM_TARGET(skeleton
    COMMENT "Assembling Skeleton..."
    COMMAND ${CMAKE_COMMAND} -DDIR=${CMAKE_BINARY_DIR}/skeleton -P ${CMAKE_SOURCE_DIR}/misc/rm_recurse.cmake
    COMMAND ${CMAKE_COMMAND} -DDIR=${CMAKE_BINARY_DIR}/skeleton -P ${CMAKE_SOURCE_DIR}/misc/mkdir.cmake
    COMMAND ${CMAKE_COMMAND} -DDIR=${CMAKE_BINARY_DIR}/skeleton/scripts -P ${CMAKE_SOURCE_DIR}/misc/mkdir.cmake
    COMMAND ${CMAKE_COMMAND} -DDIR=${CMAKE_BINARY_DIR}/skeleton/guis -P ${CMAKE_SOURCE_DIR}/misc/mkdir.cmake
    COMMAND ${CMAKE_COMMAND} -DDIR=${CMAKE_BINARY_DIR}/skeleton/data -P ${CMAKE_SOURCE_DIR}/misc/mkdir.cmake
    COMMAND ${CMAKE_COMMAND} -DFROM=${CMAKE_SOURCE_DIR}/scripts/*.lua -DTO=${CMAKE_BINARY_DIR}/skeleton/scripts -P ${CMAKE_SOURCE_DIR}/misc/cp.cmake
    COMMAND ${CMAKE_COMMAND} -DFROM=${CMAKE_SOURCE_DIR}/*mojosetupgui*.* -DTO=${CMAKE_BINARY_DIR}/skeleton/guis -P ${CMAKE_SOURCE_DIR}/misc/cp.cmake
    COMMAND ${CMAKE_COMMAND} -DFROM=${MOJOSETUP_BINARY_LOCATION} -DTO=${CMAKE_BINARY_DIR}/skeleton -P ${CMAKE_SOURCE_DIR}/misc/cp.cmake
)

ADD_DEPENDENCIES(skeleton ${MOJOSETUP_TARGETS})

# end of CMakeLists.txt ...

