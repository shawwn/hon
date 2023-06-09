#!/bin/sh

# $Id: initscript.sh,v 1.1 2009/03/18 18:44:30 Ikkyo Exp $

case "${SCRIPTDIR}" in
/*)     ;;
*)      SCRIPTDIR="`pwd`/${SCRIPTDIR}"
esac

while true
do      case "${SCRIPTDIR}" in
        */.)    SCRIPTDIR="${SCRIPTDIR%/.}";;
        *)      break;;
        esac
done

#  The script directory is supposed to be in $TOPDIR/packages/os400.

TOPDIR=`dirname "${SCRIPTDIR}"`
TOPDIR=`dirname "${TOPDIR}"`
export SCRIPTDIR TOPDIR

#  Extract the SONAME from the library makefile.

SONAME=`sed -e '/^VERSIONINFO=/!d' -e 's/^.* \([0-9]*\):.*$/\1/' -e 'q' \
                                                < "${TOPDIR}/lib/Makefile.am"`
export SONAME


################################################################################
#
#                       Tunable configuration parameters.
#
################################################################################

TARGETLIB='CURL'                # Target OS/400 program library
STATBNDDIR='CURL_A'             # Static binding directory.
DYNBNDDIR='CURL'                # Dynamic binding directory.
SRVPGM="CURL.${SONAME}"         # Service program.
TGTCCSID='500'                  # Target CCSID of objects
DEBUG='*ALL'                    # Debug level
OPTIMIZE='10'                   # Optimisation level
OUTPUT='*NONE'                  # Compilation output option.
TGTRLS='V5R2M0'                 # Target OS release

export TARGETLIB STATBNDDIR DYNBNDDIR SRVPGM TGTCCSID DEBUG OPTIMIZE OUTPUT
export TGTRLS


################################################################################

#       Need to get the version definitions.

LIBCURL_VERSION=`grep '^#define  *LIBCURL_VERSION '                     \
                        "${TOPDIR}/include/curl/curlver.h"              |
                sed 's/.*"\(.*\)".*/\1/'`
LIBCURL_VERSION_MAJOR=`grep '^#define  *LIBCURL_VERSION_MAJOR '         \
                        "${TOPDIR}/include/curl/curlver.h"              |
                sed 's/^#define  *LIBCURL_VERSION_MAJOR  *\([^ ]*\).*/\1/'`
LIBCURL_VERSION_MINOR=`grep '^#define  *LIBCURL_VERSION_MINOR '         \
                        "${TOPDIR}/include/curl/curlver.h"              |
                sed 's/^#define  *LIBCURL_VERSION_MINOR  *\([^ ]*\).*/\1/'`
LIBCURL_VERSION_PATCH=`grep '^#define  *LIBCURL_VERSION_PATCH '         \
                        "${TOPDIR}/include/curl/curlver.h"              |
                sed 's/^#define  *LIBCURL_VERSION_PATCH  *\([^ ]*\).*/\1/'`
LIBCURL_VERSION_NUM=`grep '^#define  *LIBCURL_VERSION_NUM '             \
                        "${TOPDIR}/include/curl/curlver.h"              |
                sed 's/^#define  *LIBCURL_VERSION_NUM  *0x\([^ ]*\).*/\1/'`
LIBCURL_TIMESTAMP=`grep '^#define  *LIBCURL_TIMESTAMP '                 \
                        "${TOPDIR}/include/curl/curlver.h"              |
                sed 's/.*"\(.*\)".*/\1/'`
export LIBCURL_VERSION
export LIBCURL_VERSION_MAJOR LIBCURL_VERSION_MINOR LIBCURL_VERSION_PATCH
export LIBCURL_VERSION_NUM LIBCURL_TIMESTAMP

################################################################################
#
#                       OS/400 specific definitions.
#
################################################################################

LIBIFSNAME="/QSYS.LIB/${TARGETLIB}.LIB"


################################################################################
#
#                               Procedures.
#
################################################################################

#       action_needed dest [src]
#
#       dest is an object to build
#       if specified, src is an object on which dest depends.
#
#       exit 0 (succeeds) if some action has to be taken, else 1.

action_needed()

{
        [ ! -e "${1}" ] && return 0
        [ "${2}" ] || return 1
        [ "${1}" -ot "${2}" ] && return 0
        return 1
}


#       make_module module_name source_name [additional_definitions]
#
#       Compile source name into ASCII module if needed.
#       As side effect, append the module name to variable MODULES.
#       Set LINK to "YES" if the module has been compiled.

make_module()

{
        MODULES="${MODULES} ${1}"
        MODIFSNAME="${LIBIFSNAME}/${1}.MODULE"
        action_needed "${MODIFSNAME}" "${2}" || return 0;

        #       #pragma convert has to be in the source file itself, i.e.
        #               putting it in an include file makes it only active
        #               for that include file.
        #       Thus we build a temporary file with the pragma prepended to
        #               the source file and we compile that themporary file.

        echo "#line 1 \"${2}\"" > __tmpsrcf.c
        echo "#pragma convert(819)" >> __tmpsrcf.c
        echo "#line 1" >> __tmpsrcf.c
        cat "${2}" >> __tmpsrcf.c
        CMD="CRTCMOD MODULE(${TARGETLIB}/${1}) SRCSTMF('__tmpsrcf.c')"
#       CMD="${CMD} SYSIFCOPT(*IFS64IO) OPTION(*INCDIRFIRST *SHOWINC *SHOWSYS)"
        CMD="${CMD} SYSIFCOPT(*IFS64IO) OPTION(*INCDIRFIRST)"
        CMD="${CMD} LOCALETYPE(*LOCALE)"
        CMD="${CMD} INCDIR('/qibm/proddata/qadrt/include'"
        CMD="${CMD} '${TOPDIR}/include/curl' '${TOPDIR}/include'"
        CMD="${CMD} '${TOPDIR}/packages/OS400' ${INCLUDES})"
        CMD="${CMD} TGTCCSID(${TGTCCSID}) TGTRLS(${TGTRLS})"
        CMD="${CMD} OUTPUT(${OUTPUT})"
        CMD="${CMD} OPTIMIZE(${OPTIMIZE})"
        CMD="${CMD} DBGVIEW(${DEBUG})"

        if [ "${3}" ]
        then    CMD="${CMD} DEFINE(${3})"
        fi

        system "${CMD}"
        rm -f __tmpsrcf.c
        LINK=YES
}


#       Determine DB2 object name from IFS name.

db2_name()

{
        basename "${1}"                                                 |
        tr '[a-z-]' '[A-Z_]'                                            |
        sed -e 's/\..*//'                                               \
            -e 's/^\(..........\).*/\1/'
}


#       Copy IFS file replacing version info.

versioned_copy()

{
        sed -e "s/@LIBCURL_VERSION@/${LIBCURL_VERSION}/g"               \
            -e "s/@LIBCURL_VERSION_MAJOR@/${LIBCURL_VERSION_MAJOR}/g"   \
            -e "s/@LIBCURL_VERSION_MINOR@/${LIBCURL_VERSION_MINOR}/g"   \
            -e "s/@LIBCURL_VERSION_PATCH@/${LIBCURL_VERSION_PATCH}/g"   \
            -e "s/@LIBCURL_VERSION_NUM@/${LIBCURL_VERSION_NUM}/g"       \
            -e "s/@LIBCURL_TIMESTAMP@/${LIBCURL_TIMESTAMP}/g"           \
                < "${1}" > "${2}"
}
