#!/bin/bash

set -e

scriptdir=`pwd`

INSTALL_PATH=/opt/toolchain

if [[ ! -e ${INSTALL_PATH} ]]; then
	if ! mkdir ${INSTALL_PATH}; then
		echo "Unable to create ${INSTALL_PATH}"
		exit
	fi
fi

if ! touch ${INSTALL_PATH}/test_write_permissions; then
	echo "Unable to write to ${INSTALL_PATH}"
fi
rm ${INSTALL_PATH}/test_write_permissions

system=`uname | tr a-z A-Z`
if [[ ${system:0:6} == "cygwin" ]]; then
	#tar xjf ${scriptdir}/cygwin/gcc-4.2.2-glibc-2.3.4.tar.bz2 -C ${INSTALL_PATH}
	echo "cygwin compiler outdated"
	exit 1
else
	tar xjf ${scriptdir}/linux/gcc-4.3.2-glibc-2.3.4.tar.bz2 -C ${INSTALL_PATH}
fi

for file in `ls ${scriptdir}/common/*.tar.bz2`; do
	tar xjf ${file} -C ${INSTALL_PATH}
done

echo "Install successful"
