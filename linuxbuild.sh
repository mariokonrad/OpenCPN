#!/bin/bash -ue

# -x : print every command
# -u : stop if an uninitialized variable is used
# -e : stop on error

# NOTE: this script works incorrect if the paths contain spaces

# NOTE: this script does not excessive parameter/variable checking

function usage()
{
	echo ""
	echo "usage: $0 [-h] [-r] [-c] [-v] [-i]"
	echo ""
	echo "  h : display this help"
	echo "  r : build release"
	echo "  p : build packages"
	echo "  c : just cleanup"
	echo "  v : verbose"
	echo "  i : print just info"
	echo ""
	exit 1
}

create_packages=0
cleanup=0
verbose=0
info=0

while getopts "hrpcvi" opt ; do
	case "${opt}" in
		h)
			usage
			;;
		r)
			BUILD_TYPE=Release
			;;
		p)
			create_packages=1
			;;
		c)
			cleanup=1
			;;
		v)
			verbose=1
			;;
		i)
			info=1
			verbose=1
			;;
		*)
			usage
			;;
	esac
done
shift $((OPTIND - 1))

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
if [ "${verbose}" == "1" ] ; then
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
	echo "  create packages: ${create_packages}"
	echo "  cleanup        : ${cleanup}"
	echo "  verbose        : ${verbose}"
	echo "  info           : ${info}"
	echo ""
fi

if [ "${info}" == "1" ] ; then
	exit 0
fi

# remove all directories
if [ "${cleanup}" == "1" ] ; then
	if [ -d ${INSTALL_DIR} ] ; then
		rm -fr ${INSTALL_DIR}
	fi
	if [ -d ${BUILD_DIR} ] ; then
		rm -fr ${BUILD_DIR}
	fi
	if [ -d ${DEPLOY_DIR} ] ; then
		rm -fr ${DEPLOY_DIR}
	fi
	exit 0
fi

# cleanup previous installation
if [ -d "${INSTALL_DIR}" ] ; then
	rm -fr ${INSTALL_DIR}/*
else
	mkdir -p ${INSTALL_DIR}
fi

# cleanup previous build
if [ -d "${BUILD_DIR}" ] ; then
	rm -fr ${BUILD_DIR}/*
else
	mkdir -p ${BUILD_DIR}
fi

# cleanup previous deploy
if [ -d "${DEPLOY_DIR}" ] ; then
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
if [ "${create_packages}" == "1" ] ; then
	cpack -G TGZ -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
	cpack -G ZIP -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
	cpack -G DEB -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
	if [ -x rpmbuild ] ; then
		cpack -G RPM -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
	fi
fi

exit 0
