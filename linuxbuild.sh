#!/bin/bash -xue

# -x : print every command
# -u : stop if an uninitialized variable is used
# -e : stop on error

# NOTE: this script works incorrect if the paths contain spaces

# NOTE: this script does not excessive parameter/variable checking


# gather information about the platform
script_path=$(dirname `readlink -f $0`)
cores=`grep -i 'processor' /proc/cpuinfo | wc -l`

# set necessary variables, using default values
SRC_DIR=${SRC_DIR:-${script_path}}
BUILD_DIR=${BUILD_DIR:-`pwd`/build}
INSTALL_DIR=${INSTALL_DIR:-`pwd`/install}
DEPLOY_DIR=${DEPLOY_DIR:-`pwd`/deploy}
BUILD_TYPE=${BUILD_TYPE:-Debug}
CORES=${CORES:-${cores}}

# print information
echo ""
echo "INFO:"
echo "  SRC_DIR     = ${SRC_DIR}"
echo "  BUILD_DIR   = ${BUILD_DIR}"
echo "  INSTALL_DIR = ${INSTALL_DIR}"
echo "  DEPLOY_DIR  = ${DEPLOY_DIR}"
echo ""
echo "  BUILD_TYPE  = ${BUILD_TYPE}"
echo "  CORES       = ${CORES}"
echo ""

# cleanup previous installation
if [ -d ${INSTALL_DIR} ] ; then
	rm -fr ${INSTALL_DIR}/*
else
	mkdir -p ${INSTALL_DIR}
fi

# cleanup previous build
if [ -d ${BUILD_DIR} ] ; then
	rm -fr ${BUILD_DIR}/*
else
	mkdir -p ${BUILD_DIR}
fi

# cleanup previous deploy
if [ -d ${DEPLOY_DIR} ] ; then
	rm -fr ${DEPLOY_DIR}/*
else
	mkdir -p ${DEPLOY_DIR}
fi

# build and install
cd ${BUILD_DIR}
cmake -DPREFIX=${INSTALL_DIR} -DBUILD_TYPE=${BUILD_TYPE} ${SRC_DIR}
make -j ${CORES}
make install

# build packages
cpack -G TGZ -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
cpack -G ZIP -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
cpack -G DEB -C ${BUILD_TYPE} -B ${DEPLOY_DIR}

if [ -x rpmbuild ] ; then
	cpack -G RPM -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
fi

exit 0
