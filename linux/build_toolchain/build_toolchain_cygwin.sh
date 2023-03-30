#!/bin/sh
# build a cygwin i686 -> x86_64 multilib (32/64 bit) compiler

set -ev +h

builddir=/var/tmp/build
sourcesdir=`pwd`/sources
patchesdir=`pwd`/patches

HOST="i686-cross-cygwin"
TARGET="x86_64-pc-linux-gnu"
TARGET32="i686-pc-linux-gnu"
PREFIX=/opt/toolchain/gcc-4.2.2-glibc-2.3.4/${TARGET}

# don't touch these
LC_ALL=POSIX
PATH=${PREFIX}/bin:${PATH}
SYSROOT=${PREFIX}/sys-root
HEADERDIR=${SYSROOT}/usr/include


# create the dirs that we'll be installing into
mkdir -p ${PREFIX}
mkdir -p ${SYSROOT}
mkdir -p ${HEADERDIR}
mkdir -p ${SYSROOT}/lib
mkdir -p ${SYSROOT}/lib64
mkdir -p ${SYSROOT}/usr/lib
mkdir -p ${SYSROOT}/usr/lib64
mkdir -p ${SYSROOT}/bin


# unpack and patch
mkdir -p ${builddir}
cd ${builddir}
tar xjf ${sourcesdir}/linux-2.6.23.12.tar.bz2

cd ${builddir}
tar xjf ${sourcesdir}/binutils-2.18.tar.bz2

cd ${builddir}
tar xjf ${sourcesdir}/glibc-2.3.4.tar.bz2
cd glibc-2.3.4
patch -Np1 -i ${patchesdir}/make-install-lib-all.patch
patch -Np1 -i ${patchesdir}/glibc-2.3.4-gcc4.patch
patch -Np1 -i ${patchesdir}/glibc-2.3.5-allow-gcc4-symbols.patch
patch -Np1 -i ${patchesdir}/glibc-2.3.5-allow-gcc4-string.patch
patch -Np1 -i ${patchesdir}/glibc-2.3.5-allow-gcc4-wcstol_l.patch
patch -Np1 -i ${patchesdir}/glibc-2.3.4-sem_trywait.patch
patch -Np1 -i ${patchesdir}/glibc-2.3.4-cygwin.patch

cd ${builddir}
tar xjf ${sourcesdir}/gcc-4.2.2.tar.bz2
cd gcc-4.2.2
cp -v gcc/Makefile.in{,.orig}
sed -e "s@\(^CROSS_SYSTEM_HEADER_DIR =\).*@\1 ${HEADERDIR}@g"     gcc/Makefile.in.orig > gcc/Makefile.in


# kernel headers
cd ${builddir}
cd linux-2.6.23.12
make mrproper
make ARCH=x86_64 headers_check
make ARCH=x86_64 INSTALL_HDR_PATH=dest headers_install
cp -rv dest/include/* ${HEADERDIR}


# cross binutils
cd ${builddir}
mkdir -p binutils-build
cd binutils-build
AR=ar AS=as ../binutils-2.18/configure --prefix=${PREFIX} --target=${TARGET} --host=${HOST} --with-sysroot=${SYSROOT} --disable-nls --enable-shared --enable-64-bit-bfd --disable-werror
make configure-host all install


# glibc headers
cd ${builddir}
mkdir -p glibc-build-headers
cd glibc-build-headers
CC=gcc ../glibc-2.3.4/configure --prefix=/usr --build=${BUILD} --host=${TARGET} --without-cvs --disable-sanity-checks --with-headers=${HEADERDIR} --enable-hacker-mode
make cross-compiling=yes install_root=${SYSROOT} install-headers
cp bits/stdio_lim.h $HEADERDIR/bits/stdio_lim.h
mkdir -p ${HEADERDIR}/gnu
touch ${HEADERDIR}/gnu/stubs.h


# gcc to build glibc
cd ${builddir}
mkdir -p gcc-build-core
cd gcc-build-core
../gcc-4.2.2/configure --prefix=${PREFIX} --host=${HOST} --target=${TARGET} --with-local-prefix=${SYSROOT} --disable-nls --disable-shared --disable-threads --enable-__cxa_atexit --enable-languages=c --with-sysroot=${SYSROOT} --enable-version-specific-runtime-libs
make all-gcc install-gcc


# 32 bit glibc - libraries
cd ${builddir}
mkdir -p glibc-build-32bit
cd glibc-build-32bit
export libc_cv_forced_unwind=yes libc_cv_c_cleanup=yes
BUILD_CC=gcc CFLAGS="-march=i686 -mtune=generic -O2" CC="${TARGET}-gcc -m32" AR="${TARGET}-ar" RANLIB="${TARGET}-ranlib" ../glibc-2.3.4/configure --prefix=/usr --build=${HOST} --host=${TARGET32} --without-cvs --disable-profile --disable-debug --without-gd --enable-add-ons --with-tls --enable-kernel=2.6.0 --with-__thread --with-headers=${HEADERDIR}
make LD="${TARGET}-ld" RANLIB="${TARGET}-ranlib" lib
make install_root=${SYSROOT} install-lib-all install-headers


# 64 bit glibc - libraries
cd ${builddir}
mkdir -p glibc-build-64bit
cd glibc-build-64bit
BUILD_CC=gcc CFLAGS="-mtune=generic -O2" CC="${TARGET}-gcc -m64" AR="${TARGET}-ar" RANLIB="${TARGET}-ranlib" ../glibc-2.3.4/configure --prefix=/usr --build=${HOST} --host=${TARGET} --without-cvs --disable-profile --disable-debug --without-gd --enable-add-ons --with-tls --enable-kernel=2.6.0 --with-__thread --with-headers=${HEADERDIR}
make LD="${TARGET}-ld" RANLIB="${TARGET}-ranlib" lib
make install_root=${SYSROOT} install-lib-all install-headers


# remove absolute paths in linker scripts
for file in libc.so libpthread.so libgcc_s.so; do
  for lib in lib lib64 usr/lib usr/lib64; do
    if test -f ${SYSROOT}/$lib/$file && test ! -h ${SYSROOT}/$lib/$file; then
      mv ${SYSROOT}/$lib/$file ${SYSROOT}/$lib/${file}.orig
      sed 's,/usr/lib/,,g;s,/usr/lib64/,,g;s,/lib/,,g;s,/lib64/,,g' < ${SYSROOT}/$lib/${file}.orig > ${SYSROOT}/$lib/$file
    fi
  done
done


# final gcc
cd ${builddir}
mkdir -p gcc-build-final
cd gcc-build-final
../gcc-4.2.2/configure --prefix=${PREFIX} --host=${HOST} --target=${TARGET} --with-local-prefix=${SYSROOT} --disable-nls --enable-threads=posix --enable-__cxa_atexit --enable-languages=c,c++ --with-sysroot=${SYSROOT} --enable-shared --enable-c99 --enable-long-long --enable-version-specific-runtime-libs
make all install


# add in symlinks so gcc can find them
ln -sf ../lib64/libgcc_s.so.1 ${PREFIX}/lib/gcc/${TARGET}/4.2.2/libgcc_s.so.1
ln -sf libgcc_s.so.1 ${PREFIX}/lib/gcc/${TARGET}/4.2.2/libgcc_s.so
ln -sf ../../lib/libgcc_s.so.1 ${PREFIX}/lib/gcc/${TARGET}/4.2.2/32/libgcc_s.so.1
ln -sf libgcc_s.so.1 ${PREFIX}/lib/gcc/${TARGET}/4.2.2/32/libgcc_s.so


# 32 bit glibc - everything else
cd ${builddir}/glibc-build-32bit
make LD="${TARGET}-ld" RANLIB="${TARGET}-ranlib"
make install_root=${SYSROOT} install


# 64 bit glibc - everything else
cd ${builddir}/glibc-build-64bit
make LD="${TARGET}-ld" RANLIB="${TARGET}-ranlib"
make install_root=${SYSROOT} install


# remove absolute paths in linker scripts
for file in libc.so libpthread.so libgcc_s.so; do
  for lib in lib lib64 usr/lib usr/lib64; do
    if test -f ${SYSROOT}/$lib/$file && test ! -h ${SYSROOT}/$lib/$file; then
      mv ${SYSROOT}/$lib/$file ${SYSROOT}/$lib/${file}.orig
      sed 's,/usr/lib/,,g;s,/usr/lib64/,,g;s,/lib/,,g;s,/lib64/,,g' < ${SYSROOT}/$lib/${file}.orig > ${SYSROOT}/$lib/$file
    fi
  done
done
