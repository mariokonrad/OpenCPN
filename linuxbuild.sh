#!/bin/bash -ue

# -x : print every command
# -u : stop if an uninitialized variable is used
# -e : stop on error

# NOTE: this script works incorrect if the paths contain spaces

function usage()
{
	echo ""
	echo "usage: $(basename $0) [options]"
	echo ""
	echo "Options:"
	echo "  --help       | -h  : display this help"
	echo "  --clean      | -c  : just cleanup"
	echo "  --verbose    | -v  : verbose"
	echo "  --info             : print just info"
	echo ""
	echo "Options altering the default (prepare, build, install, no packaging, not incremental):"
	echo "  --increment  | -i  : build incrementally (no prior clean)"
	echo "  --release    | -r  : build release"
	echo "  --no-bulid         : prevent building"
	echo "  --no-install       : prevent installing"
	echo "  --no-package       : prevent build packages"
	echo ""
	echo "Options for explicit execution:"
	echo "  --prepare          : equivalent of 'cmake' (prepare, no build, no install, no packaging, not incremental)"
	echo "  --make             : equivalent of 'make' (no prepare, build, no install, no packaging, incremental)"
	echo "  --install          : equivalent of 'make install' (no prepare, no build, install, no packaging, incremental)"
	echo "  --package          : equivalent of 'cpack' (no prepare, no build, no install, packaging, incremental)"
	echo ""
	echo "Options for low level control:"
	echo "  -j cores           : specify number of cores"
	echo ""
}

# gather information about the platform
script_path=$(dirname `readlink -f $0`)
cores=`grep -i 'processor' /proc/cpuinfo | wc -l`


# parse options

args=`getopt --options "hrcvij:" --longoptions help,release,package,clean,verbose,info,increment,no-install,make,prepare,install -- "$@"`
if [ $? != 0 ] ; then
	echo "Parameter error. abort." >&2
	exit 1
fi
eval set -- "${args}"

cleanup=0
verbose=0
info=0
prepare=1
build=1
install=1
create_packages=0
incremental=0

while true ; do
	case "$1" in
		--help|-h)
			usage
			exit 1
			;;
		--increment|-i)
			incremental=1
			;;
		--release|-r)
			BUILD_TYPE=Release
			;;
		--clean|-c)
			cleanup=1
			;;
		--verbose|-v)
			verbose=1
			;;
		--info)
			info=1
			verbose=1
			;;
		--no-build)
			build=0
			;;
		--no-install)
			install=0
			;;
		--no-package)
			create_packages=0
			;;
		--prepare)
			prepare=1
			build=0
			install=0
			create_packages=0
			incremental=0
			;;
		--make)
			prepare=0
			build=1
			install=0
			create_packages=0
			incremental=1
			;;
		--install)
			prepare=0
			build=0
			install=1
			create_packages=0
			incremental=1
			;;
		--package)
			prepare=0
			build=0
			install=0
			create_packages=1
			incremental=1
			;;
		-j)
			cores=$2
			shift
			;;
		--)
			break
			;;
		*)
			echo "Internal error. abort." >&2
			exit 1
			;;
	esac
	shift
done

# ensure lower bound of cores
if [ ${cores} -lt 1 ] ; then
	cores=1
fi

# set necessary variables, using default values
SRC_DIR=${SRC_DIR:-${script_path}}
BUILD_DIR=${BUILD_DIR:-`pwd`/build}
INSTALL_DIR=${INSTALL_DIR:-`pwd`/install}
DEPLOY_DIR=${DEPLOY_DIR:-`pwd`/deploy}
BUILD_TYPE=${BUILD_TYPE:-Debug}
CORES=${CORES:-${cores}}
CURRENT_DIR=`pwd`

# print information
if [ ${verbose} -ne 0 ] ; then
	echo ""
	echo "INFO:"
	echo "  SRC_DIR     = ${SRC_DIR}"
	echo "  BUILD_DIR   = ${BUILD_DIR}"
	echo "  INSTALL_DIR = ${INSTALL_DIR}"
	echo "  DEPLOY_DIR  = ${DEPLOY_DIR}"
	echo ""
	echo "  CURRENT_DIR = ${CURRENT_DIR}"
	echo ""
	echo "  BUILD_TYPE  = ${BUILD_TYPE}"
	echo "  CORES       = ${CORES}"
	echo ""
	echo "  incremental    : ${incremental}"
	echo "  cleanup        : ${cleanup}"
	echo "  verbose        : ${verbose}"
	echo "  info           : ${info}"
	echo "  prepare        : ${prepare}"
	echo "  build          : ${build}"
	echo "  install        : ${install}"
	echo "  create packages: ${create_packages}"
	echo ""
fi

if [ ${info} -ne 0 ] ; then
	exit 0
fi

# remove all directories
if [ ${cleanup} -ne 0 ] ; then
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

if [ ${incremental} -eq 0 ] ; then
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
fi

function check_build_dir()
{
	if [ ! -d "${BUILD_DIR}" ] ; then
		echo "ERROR: build directory does not exist. abort." >&2
		exit 1
	fi
}

function check_install_dir()
{
	if [ ! -d "${INSTALL_DIR}" ] ; then
		echo "ERROR: install directory does not exist. abort." >&2
		exit 1
	fi
}

function check_deploy_dir()
{
	if [ ! -d "${DEPLOY_DIR}" ] ; then
		echo "ERROR: deploy directory does not exist. abort." >&2
		exit 1
	fi
}

function exec_prepare()
{
	if [ ${prepare} -eq 0 ] ; then
		return
	fi

	check_build_dir
	cd ${BUILD_DIR}

	cmake -DPREFIX=${INSTALL_DIR} -DBUILD_TYPE=${BUILD_TYPE} ${SRC_DIR}

	cd ${CURRENT_DIR}
}

function exec_build()
{
	if [ ${build} -eq 0 ] ; then
		return
	fi

	check_build_dir
	cd ${BUILD_DIR}

	make -j ${CORES}

	cd ${CURRENT_DIR}
}

function exec_install()
{
	if [ ${install} -eq 0 ] ; then
		return
	fi

	check_build_dir
	check_install_dir
	cd ${BUILD_DIR}

	make install

	cd ${CURRENT_DIR}
}

function exec_packaging()
{
	if [ ${create_packages} -eq 0 ] ; then
		return
	fi

	check_build_dir
	check_deploy_dir
	cd ${BUILD_DIR}

	cpack -G TGZ -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
	cpack -G ZIP -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
	cpack -G DEB -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
	if [ -x rpmbuild ] ; then
		cpack -G RPM -C ${BUILD_TYPE} -B ${DEPLOY_DIR}
	fi

	cd ${CURRENT_DIR}
}

exec_prepare
exec_build
exec_install
exec_packaging

exit 0
