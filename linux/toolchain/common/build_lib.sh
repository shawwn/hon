#!/bin/bash

set -e

SOIL_VERSION=
FMOD_VERSION=42807-s2
LIBPNG_VERSION=1.4.1
FREETYPE_VERSION=2.3.7
SPEEX_VERSION=1.2rc1
OPENSSL_VERSION=0.9.8n
CURL_VERSION=7.19.6
NCURSES_VERSION=5.6
JPEG_VERSION=8
BOOST_VERSION=1.41.0
GIFLIB_VERSION=4.1.6

scriptdir=`pwd`
builddir=/var/tmp/build
sourcesdir=${scriptdir}/sources
parallelmake="-j6"

TOOLCHAIN=gcc-4.3.2-glibc-2.3.4
TARGET="x86_64-unknown-linux-gnu"

PATH=/opt/toolchain/${TOOLCHAIN}/${TARGET}/bin:${PATH}
CFLAGS32="-O3 -s -m32 -march=i686 -mtune=generic -mmmx"
CFLAGS64="-O3 -s -m64 -mtune=generic -mmmx -msse -msse2 -fPIC"
ARFLAGS32="-m32"
ARFLAGS64="-m64"
LDFLAGS32="-O3 -s -m32"
LDFLAGS64="-O3 -s -m64"

SOIL_SOURCE=soil.zip
FMOD_SOURCE=fmodapi${FMOD_VERSION}linux.tar.gz
FMOD64_SOURCE=fmodapi${FMOD_VERSION}linux64.tar.gz
LIBPNG_SOURCE=libpng-${LIBPNG_VERSION}.tar.bz2
FREETYPE_SOURCE=freetype-${FREETYPE_VERSION}.tar.bz2
SPEEX_SOURCE=speex-${SPEEX_VERSION}.tar.gz
OPENSSL_SOURCE=openssl-${OPENSSL_VERSION}.tar.gz
CURL_SOURCE=curl-${CURL_VERSION}.tar.bz2
NCURSES_SOURCE=ncurses-${NCURSES_VERSION}.tar.gz
JPEG_SOURCE=jpegsrc.v${JPEG_VERSION}.tar.gz
BOOST_SOURCE=boost_${BOOST_VERSION//./_}.tar.bz2
GIFLIB_SOURCE=giflib-${GIFLIB_VERSION}.tar.bz2

SOIL_DOWNLOAD=http://www.lonesock.net/files/soil.zip
FMOD_DOWNLOAD=http://www.fmod.org/index.php/release/version/${FMOD_SOURCE}
FMOD64_DOWNLOAD=http://www.fmod.org/index.php/release/version/${FMOD64_SOURCE}
LIBPNG_DOWNLOAD=ftp://ftp.simplesystems.org/pub/libpng/png/src/${LIBPNG_SOURCE}
FREETYPE_DOWNLOAD=http://download.savannah.gnu.org/releases/freetype/${FREETYPE_SOURCE}
SPEEX_DOWNLOAD=http://downloads.xiph.org/releases/speex/${SPEEX_SOURCE}
OPENSSL_DOWNLOAD=http://www.openssl.org/source/${OPENSSL_SOURCE}
CURL_DOWNLOAD=http://curl.haxx.se/download/${CURL_SOURCE}
NCURSES_DOWNLOAD=http://ftp.gnu.org/pub/gnu/ncurses/${NCURSES_SOURCE}
JPEG_DOWNLOAD=http://www.ijg.org/files/${JPEG_SOURCE}
BOOST_DOWNLOAD=http://downloads.sourceforge.net/project/boost/boost/${BOOST_VERSION}/${BOOST_SOURCE}
GIFLIB_DOWNLOAD=http://downloads.sourceforge.net/project/giflib/giflib%204.x/giflib-${GIFLIB_VERSION}/${GIFLIB_SOURCE}

SYSROOT=${TOOLCHAIN}/${TARGET}/${TARGET}/sys-root

rebuild=0

function get_and_unpack ()
{
	source=${1}
	download=${2}
	
	if [[ ${download:0:4} == "http" || ${download:0:3} == "ftp" ]]; then
		if [[ ! -e ${sourcesdir}/${source} ]]; then
			wget -P ${sourcesdir} ${download}
		fi
	else
		# source is directory in local filesystem
		cp -ra ${source} ${builddir}
	fi
	
	if [[ ${source:((${#source}-7)):7} == "tar.bz2" ]]; then
		tar xjf ${sourcesdir}/${source} -C ${builddir}
	elif [[ ${source:((${#source}-6)):6} == "tar.gz" ]]; then
		tar xzf ${sourcesdir}/${source} -C ${builddir}
	elif [[ ${source:((${#source}-3)):3} == "zip" ]]; then
		unzip -q ${sourcesdir}/${source} -d ${builddir}
	fi
}

buildme=0
function needs_build ()
{
	package=${1}
	if [[ -e "${scriptdir}/${package}" ]]; then
		if [[ ${rebuild} == 1 ]]; then
			rm "${scriptdir}/${package}"
		else
			echo "${package} exists.  Use --rebuild to rebuild it."
			buildme=0
			return
		fi
	fi
	buildme=1
}

# can be used to build any configure based library
# arg1 = name of the lib
# arg2 = extra params to pass to configure
# others = extra commands to be executed after building, but before creating the tarball
function build ()
{
	name=${1}
	upname=`echo ${name} | tr a-z A-Z`
	extopts=${2}
	
	version=${upname}_VERSION
	eval version=\$${version}
	
	source=${upname}_SOURCE
	eval source=\$$source
	
	download=${upname}_DOWNLOAD
	eval download=\$$download
	
	needs_build ${name}-${version}.tar.bz2
	if [[ ${buildme} == 0 ]]; then
		return
	fi
	
	echo Creating ${name}-${version}.tar.bz2
	
	get_and_unpack ${source} ${download}
	mkdir -p ${builddir}/build-${name}32
	mkdir -p ${builddir}/build-${name}64
	
	cd ${builddir}/build-${name}32
	CFLAGS=${CFLAGS32} LDFLAGS=${LDFLAGS32} ../${name}-${version}/configure --build=${TARGET} --host=${TARGET} --prefix=${builddir}/deleteme --libdir=${builddir}/${SYSROOT}/lib --includedir=${builddir}/${SYSROOT}/usr/include ${extopts}
	make ${parallelmake}
	make install
	rm -rf ${builddir}/${SYSROOT}/lib/{*.a,*.la,pkgconfig}
	
	cd ${builddir}/build-${name}64
	CFLAGS=${CFLAGS64} LDFLAGS=${LDFLAGS64} ../${name}-${version}/configure --build=${TARGET} --host=${TARGET} --prefix=${builddir}/deleteme --libdir=${builddir}/${SYSROOT}/lib64 --includedir=${builddir}/${SYSROOT}/usr/include ${extopts}
	make ${parallelmake}
	make install
	rm -rf ${builddir}/${SYSROOT}/lib64/{*.a,*.la,pkgconfig}
	
	while [[ ${3}x != x ]]; do
		eval ${3}
		shift
	done
	
	if [[ "${name}" == "curl" ]]; then
		sed -i -e 's/#define CURL_SIZEOF_LONG 8/#ifdef __x86_64__\
#define CURL_SIZEOF_LONG 8\
#else\
#define CURL_SIZEOF_LONG 4\
#endif/' -e 's/#define CURL_SIZEOF_CURL_OFF_T 8/#ifdef __x86_64__\
#define CURL_SIZEOF_CURL_OFF_T 8\
#else\
#define CURL_SIZEOF_CURL_OFF_T 4\
#endif/' ${builddir}/${SYSROOT}/usr/include/curl/curlbuild.h
	fi
	
	cd ${scriptdir}
	tar cjf ${name}-${version}.tar.bz2 -C ${builddir} ${TOOLCHAIN}
	echo "${name}-${version}.tar.bz2 created"
	
	rm -rf ${builddir}/${TOOLCHAIN} ${builddir}/deleteme ${builddir}/build-${name}32 ${builddir}/build-${name}64 ${builddir}/${name}-${version}
}

# can be used to build any configure based library
# arg1 = name of the lib
# arg2 = extra params to pass to configure
# others = extra commands to be executed after building, but before creating the tarball
function build_static ()
{
	name=${1}
	upname=`echo ${name} | tr a-z A-Z`
	extopts=${2}
	
	version=${upname}_VERSION
	eval version=\$${version}
	
	source=${upname}_SOURCE
	eval source=\$$source
	
	download=${upname}_DOWNLOAD
	eval download=\$$download
	
	needs_build ${name}-${version}.tar.bz2
	if [[ ${buildme} == 0 ]]; then
		return
	fi
	
	echo Creating ${name}-${version}.tar.bz2
	
	get_and_unpack ${source} ${download}
	mkdir -p ${builddir}/build-${name}32
	mkdir -p ${builddir}/build-${name}64
	
	cd ${builddir}/build-${name}32
	CFLAGS=${CFLAGS32} LDFLAGS=${LDFLAGS32} ../${name}-${version}/configure --build=${TARGET} --host=${TARGET} --prefix=${builddir}/deleteme --libdir=${builddir}/${SYSROOT}/lib --includedir=${builddir}/${SYSROOT}/usr/include ${extopts}
	make ${parallelmake}
	make install
	rm -rf ${builddir}/${SYSROOT}/lib/{*.so*,*.la,pkgconfig}
	
	cd ${builddir}/build-${name}64
	CFLAGS=${CFLAGS64} LDFLAGS=${LDFLAGS64} ../${name}-${version}/configure --build=${TARGET} --host=${TARGET} --prefix=${builddir}/deleteme --libdir=${builddir}/${SYSROOT}/lib64 --includedir=${builddir}/${SYSROOT}/usr/include ${extopts}
	make ${parallelmake}
	make install
	rm -rf ${builddir}/${SYSROOT}/lib64/{*.so*,*.la,pkgconfig}
	
	while [[ ${3}x != x ]]; do
		eval ${3}
		shift
	done
	
	if [[ "${name}" == "curl" ]]; then
		sed -i -e 's/#define CURL_SIZEOF_LONG 8/#ifdef __x86_64__\
#define CURL_SIZEOF_LONG 8\
#else\
#define CURL_SIZEOF_LONG 4\
#endif/' -e 's/#define CURL_SIZEOF_CURL_OFF_T 8/#ifdef __x86_64__\
#define CURL_SIZEOF_CURL_OFF_T 8\
#else\
#define CURL_SIZEOF_CURL_OFF_T 4\
#endif/' ${builddir}/${SYSROOT}/usr/include/curl/curlbuild.h
	fi
	
	cd ${scriptdir}
	tar cjf ${name}-${version}.tar.bz2 -C ${builddir} ${TOOLCHAIN}
	echo "${name}-${version}.tar.bz2 created"
	
	rm -rf ${builddir}/${TOOLCHAIN} ${builddir}/deleteme ${builddir}/build-${name}32 ${builddir}/build-${name}64 ${builddir}/${name}-${version}
}

function build_soil ()
{
	needs_build soil.tar.bz2
	if [[ ${buildme} == 0 ]]; then
		return
	fi
	
	echo "Creating soil.tar.bz2"
	
	get_and_unpack ${SOIL_SOURCE} ${SOIL_DOWNLOAD}
	
	mkdir -p ${builddir}/${SYSROOT}/lib
	mkdir -p ${builddir}/${SYSROOT}/lib64
	mkdir -p ${builddir}/${SYSROOT}/usr/include
	
	cd "${builddir}/Simple OpenGL Image Library/src"
	
	cp SOIL.h ${builddir}/${SYSROOT}/usr/include
	
	rm -f image_helper.o stb_image_aug.o image_DXT.o SOIL.o libSOIL.a
	${TARGET}-gcc image_helper.c stb_image_aug.c image_DXT.c SOIL.c -c ${CFLAGS32}
	${TARGET}-ar rcs libSOIL.a image_helper.o stb_image_aug.o image_DXT.o SOIL.o
	cp libSOIL.a ${builddir}/${SYSROOT}/lib
	
	rm -f image_helper.o stb_image_aug.o image_DXT.o SOIL.o libSOIL.a
	${TARGET}-gcc image_helper.c stb_image_aug.c image_DXT.c SOIL.c -c ${CFLAGS64}
	${TARGET}-ar rcs libSOIL.a image_helper.o stb_image_aug.o image_DXT.o SOIL.o
	cp libSOIL.a ${builddir}/${SYSROOT}/lib64
	
	cd ${scriptdir}
	tar cjf soil.tar.bz2 -C ${builddir} ${TOOLCHAIN}
	echo "soil.tar.bz2 created"
	
	rm -rf ${builddir}/${TOOLCHAIN} "${builddir}/Simple OpenGL Image Library"
}

function build_fmod ()
{
	needs_build fmod-${FMOD_VERSION}.tar.bz2
	if [[ ${buildme} == 0 ]]; then
		return
	fi
	
	echo "Creating fmod-${FMOD_VERSION}.tar.bz2"
	
	get_and_unpack ${FMOD_SOURCE} ${FMOD_DOWNLOAD}
	get_and_unpack ${FMOD64_SOURCE} ${FMOD64_DOWNLOAD}
	
	mkdir -p ${builddir}/${SYSROOT}/lib
	mkdir -p ${builddir}/${SYSROOT}/lib64
	mkdir -p ${builddir}/${SYSROOT}/usr/include
	
	cd ${builddir}/fmodapi${FMOD_VERSION}linux/api
	cp -a lib/libfmodex.so* ${builddir}/${SYSROOT}/lib
	cp -a lib/libfmodex-*.so ${builddir}/${SYSROOT}/lib
	cp inc/fmod*.h inc/fmod.hpp ${builddir}/${SYSROOT}/usr/include
	
	cd ${builddir}/fmodapi${FMOD_VERSION}linux64/api
	cp -a lib/libfmodex64.so* ${builddir}/${SYSROOT}/lib64
	cp -a lib/libfmodex64-*.so ${builddir}/${SYSROOT}/lib64
	
	cd ${scriptdir}
	tar cjf fmod-${FMOD_VERSION}.tar.bz2 -C ${builddir} ${TOOLCHAIN}
	echo "fmod-${FMOD_VERSION}.tar.bz2 created"
	
	rm -rf ${builddir}/${TOOLCHAIN} ${builddir}/fmodapi${FMOD_VERSION}linux ${builddir}/fmodapi${FMOD_VERSION}linux64
}

function build_openssl ()
{
	name=openssl
	upname=`echo ${name} | tr a-z A-Z`
	
	version=${upname}_VERSION
	eval version=\$${version}
	
	source=${upname}_SOURCE
	eval source=\$$source
	
	download=${upname}_DOWNLOAD
	eval download=\$$download
	
	needs_build ${name}-${version}.tar.bz2
	if [[ ${buildme} == 0 ]]; then
		return
	fi
	
	echo Creating ${name}-${version}.tar.bz2
	
	mkdir -p ${builddir}/${SYSROOT}/lib
	mkdir -p ${builddir}/${SYSROOT}/lib64
	mkdir -p ${builddir}/${SYSROOT}/usr/include/openssl
	
	get_and_unpack ${source} ${download}
	
	cd ${builddir}/${name}-${version}
	./Configure linux-elf --prefix=${builddir}/${SYSROOT}/usr --openssldir=${builddir}/${SYSROOT}/usr/openssl shared
	CC="/opt/toolchain/${TOOLCHAIN}/${TARGET}/bin/${TARGET}-gcc -m32" AR="/opt/toolchain/${TOOLCHAIN}/${TARGET}/bin/${TARGET}-ar rcs" RANLIB="/opt/toolchain/${TOOLCHAIN}/${TARGET}/bin/${TARGET}-ranlib" make -e
	cp ${builddir}/${name}-${version}/*.a  ${builddir}/${SYSROOT}/lib
	cp ${builddir}/${name}-${version}/include/openssl/*.h ${builddir}/${SYSROOT}/usr/include/openssl
	rm -rf ${builddir}/${name}-${version}
	
	get_and_unpack ${source} ${download}
	
	cd ${builddir}/${name}-${version}
	./Configure linux-x86_64 --prefix=${builddir}/${SYSROOT}/usr --openssldir=${builddir}/${SYSROOT}/usr/openssl no-asm # having relocation issues with the asm
	CC="/opt/toolchain/${TOOLCHAIN}/${TARGET}/bin/${TARGET}-gcc -fPIC" AR="/opt/toolchain/${TOOLCHAIN}/${TARGET}/bin/${TARGET}-ar rcs" RANLIB="/opt/toolchain/${TOOLCHAIN}/${TARGET}/bin/${TARGET}-ranlib" make -e
	cp ${builddir}/${name}-${version}/*.a  ${builddir}/${SYSROOT}/lib64
	
	cd ${scriptdir}
	tar cjf ${name}-${version}.tar.bz2 -C ${builddir} ${TOOLCHAIN}
	echo "${name}-${version}.tar.bz2 created"
	
	rm -rf ${builddir}/${TOOLCHAIN} ${builddir}/${name}-${version}
}

function build_speedtree
{
	needs_build speedtree.tar.bz2
	if [[ ${buildme} == 0 ]]; then
		return
	fi
	
	echo Creating speedtree.tar.bz2
	
	mkdir -p ${builddir}/${SYSROOT}/lib
	mkdir -p ${builddir}/${SYSROOT}/lib64
	mkdir -p ${builddir}/${SYSROOT}/usr/include
	
	get_and_unpack "${scriptdir}/../../../lib/SpeedTree/source"

	cd ${builddir}/source
	
	AR=${TARGET}-ar CXX=${TARGET}-g++ CXXFLAGS="-O0 -g3 -m32 -fvisibility=hidden -fvisibility-inlines-hidden" OBJ_DIR=i686-debug LIB_DIR=${builddir}/${SYSROOT}/lib make ${parallelmake} SpeedTreeRT_debug
	AR=${TARGET}-ar CXX=${TARGET}-gcc CXXFLAGS="-O0 -g3 -m64 -fvisibility=hidden -fvisibility-inlines-hidden -fPIC" OBJ_DIR=x86_64-debug LIB_DIR=${builddir}/${SYSROOT}/lib64 make ${parallelmake} SpeedTreeRT_debug
	AR=${TARGET}-ar CXX=${TARGET}-g++ CXXFLAGS="-O3 -g3  -fomit-frame-pointer -fno-strict-aliasing -m32 -fvisibility=hidden -fvisibility-inlines-hidden -march=i686 -mtune=generic -mmmx" OBJ_DIR=i686-release LIB_DIR=${builddir}/${SYSROOT}/lib make ${parallelmake} SpeedTreeRT
	AR=${TARGET}-ar CXX=${TARGET}-g++ CXXFLAGS="-O3 -g3  -fomit-frame-pointer -fno-strict-aliasing -m64 -fvisibility=hidden -fvisibility-inlines-hidden -mtune=generic -mmmx -msse -msse2 -fPIC" OBJ_DIR=x86_64-release LIB_DIR=${builddir}/${SYSROOT}/lib64 make ${parallelmake} SpeedTreeRT
	cp SourceCode/SpeedTreeRT.h ${builddir}/${SYSROOT}/usr/include
	
	cd ${scriptdir}
	tar cjf speedtree.tar.bz2 -C ${builddir} ${TOOLCHAIN}
	echo "speedtree.tar.bz2 created"
	
	rm -rf ${builddir}/${TOOLCHAIN} ${builddir}/${name}-${version//./_}
}

function build_boost
{
	# doesn't build the non-header-only libs
	name=boost
	upname=`echo ${name} | tr a-z A-Z`
	extopts=${2}
	
	version=${upname}_VERSION
	eval version=\$${version}
	
	source=${upname}_SOURCE
	eval source=\$$source
	
	download=${upname}_DOWNLOAD
	eval download=\$$download
	
	needs_build ${name}-${version}.tar.bz2
	if [[ ${buildme} == 0 ]]; then
		return
	fi
	
	echo Creating ${name}-${version}.tar.bz2
	
	get_and_unpack ${source} ${download}
	
	mkdir -p ${builddir}/${SYSROOT}/usr/include
	cp -rv ${builddir}/${name}_${version//./_}/${name} ${builddir}/${SYSROOT}/usr/include
	
	tar cjf ${name}-${version}.tar.bz2 -C ${builddir} ${TOOLCHAIN}
	echo "${name}-${version}.tar.bz2 created"
	
	rm -rf ${builddir}/${TOOLCHAIN} ${builddir}/source
}

dosoil=0
dofmod=0
dolibpng=0
dofreetype=0
dospeex=0
doopenssl=0
docurl=0
dospeedtree=0
doncurses=0
dojpeg=0
doboost=0
dogif=0

while [[ ${1}x != x ]]; do
	case "${1}" in
		"soil"		) dosoil=1;;
		"fmod"		) dofmod=1;;
		"libpng"	) dolibpng=1;;
		"freetype"	) dofreetype=1;;
		"speex"		) dospeex=1;;
		"openssl"	) doopenssl=1;;
		"curl"		) docurl=1;;
		"speedtree"	) dospeedtree=1;;
		"ncurses"	) doncurses=1;;
		"jpeg"		) dojpeg=1;;
		"boost"		) doboost=1;;
		"giflib"	) dogif=1;;
		"--rebuild"	) rebuild=1;;
	esac
	shift
done

if [[ ${dosoil} == 0 && ${dofmod} == 0 && ${dolibpng} == 0 && ${dofreetype} == 0 && ${dospeex} == 0 && ${doopenssl} == 0 && ${docurl} == 0 && ${dospeedtree} == 0 && ${doncurses} == 0 && ${dojpeg} == 0 && ${doboost} == 0 && ${dogif} == 0 ]]; then
	echo "Usage:"
	echo "  ${0} <targets> [--rebuild]"
	echo "Available targets:"
	echo "  soil"
	echo "  fmod"
	echo "  libpng"
	echo "  freetype"
	echo "  speex"
	echo "  openssl"
	echo "  curl"
	echo "  speedtree"
	echo "  ncurses"
	echo "  jpeg"
	echo "  giflib"
	exit 1
fi
	

if [[ ${dosoil} == 1 ]]; then
	build_soil
fi
if [[ ${dofmod} == 1 ]]; then
	build_fmod
fi
if [[ ${dolibpng} == 1 ]]; then
	build libpng --without-libpng-compat "sed -i -e s@__pngconf@//__pngconf@ -e s@__dont@//__dont@ ${builddir}/${SYSROOT}/usr/include/libpng14/pngconf.h"
fi
if [[ ${dofreetype} == 1 ]]; then
	build freetype "" "ln -s freetype2/freetype ${builddir}/${SYSROOT}/usr/include/freetype"
fi
if [[ ${dospeex} == 1 ]]; then
	build speex
fi
if [[ ${doopenssl} == 1 ]]; then
	build_openssl
fi
if [[ ${docurl} == 1 ]]; then
	build curl
fi
if [[ ${dospeedtree} == 1 ]]; then
	build_speedtree
fi
if [[ ${doncurses} == 1 ]]; then
	build ncurses "--with-shared --without-cxx-binding"
fi
if [[ ${dojpeg} == 1 ]]; then
	build_static jpeg
fi
if [[ ${doboost} == 1 ]]; then
	build_boost
fi
if [[ ${dogif} == 1 ]]; then
	build_static giflib "--disable-x11"
fi
