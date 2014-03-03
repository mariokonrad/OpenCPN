#!/bin/bash -ue

# -x : print every command
# -u : stop if an uninitialized variable is used
# -e : stop on error

# NOTE: this script works incorrect if the paths contain spaces

script_path=$(dirname `readlink -f $0`)
script_name=$(basename `readlink -f $0`)

function usage()
{
	echo ""
	echo "usage: ${script_name} [options]"
	echo ""
	echo "Options:"
	echo "   --help       | -h  : display this help"
	echo "   --clean      | -c  : just cleanup"
	echo "   --verbose    | -v  : verbose"
	echo "   --info             : print just info"
	echo "   --cppcheck         : runs cppcheck on the entire core"
	echo "   --understand       : creates a database for SciTools Understand"
	echo "   --index            : creates cscope/ctags index"
	echo ""
	echo "Options altering the default (prepare, build, install, no packaging, not incremental):"
	echo "   --increment  | -i  : build incrementally (no prior clean)"
	echo "   --release    | -r  : build release"
	echo "   --no-bulid         : prevent building"
	echo "   --no-unittest      : prevent unit testing"
	echo "   --no-install       : prevent installing"
	echo "   --no-package       : prevent build packages"
	echo ""
	echo "Options for explicit execution:"
	echo "   --prepare          : equivalent of 'cmake' (prepare, no build, no test, no install, no packaging, not incremental)"
	echo "   --make             : equivalent of 'make' (no prepare, build, test, no install, no packaging, incremental)"
	echo "   --test             : equivalent of 'make unittest' (no prepare, no build, test, no install, no packaging, incremental)"
	echo "   --install          : equivalent of 'make install' (no prepare, no build, no test, install, no packaging, incremental)"
	echo "   --package          : equivalent of 'cpack' (no prepare, no build, no test, no install, packaging, incremental)"
	echo ""
	echo "Options for low level control:"
	echo "   -j cores           : specify number of cores"
	echo ""
}

function cleanup_index()
{
	if [ -r "tags" ] ; then
		rm -f tags
	fi
	if [ -r "cscope.files" ] ; then
		rm -f cscope.files
	fi
	if [ -r "cscope.out" ] ; then
		rm -f cscope.out
	fi
}

function exec_create_index()
{
	cleanup_index

	for dn in src ; do
			find ${dn} -type f -regextype posix-egrep -regex ".*\.(cpp|cc|cxx|c|hpp|hh|hxx|h)" >> cscope.files
	done

	ctags -L cscope.files
	cscope -b
}

function exec_info()
{
	if [ ${opt_verbose} -eq 0 ] ; then
		return
	fi

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
	echo "  cppcheck    = `which cppcheck`"
	echo ""
	echo "  incremental    : ${opt_incremental}"
	echo "  cleanup        : ${opt_cleanup}"
	echo "  verbose        : ${opt_verbose}"
	echo "  info           : ${opt_info}"
	echo "  prepare        : ${opt_prepare}"
	echo "  build          : ${opt_build}"
	echo "  unittest       : ${opt_unittest}"
	echo "  install        : ${opt_install}"
	echo "  create packages: ${opt_create_packages}"
	echo "  cppcheck       : ${opt_cppcheck}"
	echo "  understand     : ${opt_understand}"
	echo ""
}


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
	if [ ${opt_prepare} -eq 0 ] ; then
		return
	fi

	check_build_dir
	cd ${BUILD_DIR}

	cmake \
		-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
		-DPREFIX=${INSTALL_DIR} \
		-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		${SRC_DIR}

	cd ${CURRENT_DIR}
}

function exec_cppcheck()
{
	if [ ! -x `which cppcheck` ] ; then
		echo "error: cppcheck not found. abort."
		exit 1
	fi

	cppcheck \
		-Isrc \
		--enable=all \
		--std=c++03 --std=posix \
		--platform=unix64 \
		--language=c++ \
		--force \
		-DocpnUSE_GL \
		-D__WXGTK__ \
		-D__POSIX__ \
		-U__WXMSW__ \
		-U__WXX11__ \
		-U__WXOSX__ \
		-U__WXMAC__ \
		$(find src -name "*.cpp" -o -name "*.h" -type f)
}

function exec_understand()
{
	database=${BUILD_DIR}/opencpn.udb
	binary=$HOME/bin/scitools/bin/linux64/und

	if [ ! -x "${binary}" ] ; then
		echo "error: Understand binary (${binary}) not found. abort."
		exit 1
	fi

	rm -f ${database}

	${binary} -db ${database} \
		create -languages C++ \
		settings -C++AddFoundFilesToProject on \
		settings -C++MacrosAdd VERSION="1" \
		settigns -C++IncludesAdd "/usr/include" \
		settigns -C++IncludesAdd "/usr/include/linux" \
		settings -C++IncludesAdd "/usr/include/x86_64-linux-gnu" \
		settings -C++IncludesAdd "/usr/include/x86_64-linux-gnu/c++/4.7" \
		settings -C++IncludesAdd "/usr/include/c++/4.7" \
		settings -C++IncludesAdd "/usr/include/c++/4.7/tr1" \
		settings -MetricAdd "Cyclomatic" \
		settings -MetricAdd "MaxNesting" \
		settings -MetricAdd "CountClassCoupled" \
		settings -MetricAdd "CountLineCode" \
		settings -MetricAdd "CountLineCodeExe" \
		settings -MetricAdd "CountDeclClass" \
		settings -MetricAdd "CountDeclMethodFriend" \
		settings -MetricAdd "CountDeclMethodPublic" \
		settings -MetricAdd "CountDeclInstanceMethod" \
		settings -MetricAdd "CountDeclInstanceVariable" \
		add ${BUILD_DIR}/compile_commands.json \
		analyze
}

function exec_build()
{
	if [ ${opt_build} -eq 0 ] ; then
		return
	fi

	check_build_dir
	cd ${BUILD_DIR}

	make -j ${CORES}

	cd ${CURRENT_DIR}
}

function exec_unittest()
{
	if [ ${opt_unittest} -eq 0 ] ; then
		return
	fi

	check_build_dir
	cd ${BUILD_DIR}

	make unittest

	cd ${CURRENT_DIR}
}

function exec_install()
{
	if [ ${opt_install} -eq 0 ] ; then
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
	if [ ${opt_create_packages} -eq 0 ] ; then
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


# gather information about the platform
cores=`grep -i 'processor' /proc/cpuinfo | wc -l`

# parse options

args=$(getopt \
	--options "hrcvij:" \
	--longopt "help" \
	--longopt "release" \
	--longopt "package" \
	--longopt "clean" \
	--longopt "verbose" \
	--longopt "info" \
	--longopt "index" \
	--longopt "increment" \
	--longopt "no-build" \
	--longopt "no-unittest" \
	--longopt "no-install" \
	--longopt "no-package" \
	--longopt "make" \
	--longopt "test" \
	--longopt "prepare" \
	--longopt "install" \
	--longopt "cppcheck" \
	--longopt "understand" \
	-- "$@")
if [ $? != 0 ] ; then
	echo "Parameter error. abort." >&2
	exit 1
fi
eval set -- "${args}"

opt_cleanup=0
opt_verbose=0
opt_info=0
opt_prepare=1
opt_build=1
opt_unittest=1
opt_install=1
opt_create_packages=0
opt_incremental=0
opt_cppcheck=0
opt_understand=0
opt_actions=()

function add_action()
{
	opt_actions[${#opt_actions[*]}]=$1
}

while [ $# -ne 0 ] ; do
	case "$1" in
		--help|-h)
			usage
			exit 1
			;;
		--increment|-i)
			opt_incremental=1
			;;
		--release|-r)
			BUILD_TYPE=Release
			;;
		--clean|-c)
			add_action "clean"
			opt_cleanup=1
			;;
		--verbose|-v)
			opt_verbose=1
			;;
		--info)
			add_action "info"
			opt_info=1
			opt_verbose=1
			;;
		--cppcheck)
			add_action "cppcheck"
			opt_cppcheck=1
			;;
		--understand)
			add_action "understand"
			opt_prepare=1
			opt_understand=1
			;;
		--index)
			add_action "index"
			exec_create_index
			exit 0
			;;
		--no-build)
			opt_build=0
			;;
		--no-unittest)
			opt_unittest=0
			;;
		--no-install)
			opt_install=0
			;;
		--no-package)
			opt_create_packages=0
			;;
		--prepare)
			add_action "prepare"
			opt_prepare=1
			opt_build=0
			opt_unittest=0
			opt_install=0
			opt_create_packages=0
			opt_incremental=0
			;;
		--make)
			add_action "make"
			opt_prepare=0
			opt_build=1
			opt_unittest=1
			opt_install=0
			opt_create_packages=0
			opt_incremental=1
			;;
		--test)
			add_action "test"
			opt_prepare=0
			opt_build=0
			opt_unittest=1
			opt_install=0
			opt_create_packages=0
			opt_incremental=1
			;;
		--install)
			add_action "install"
			opt_prepare=0
			opt_build=0
			opt_unittest=0
			opt_install=1
			opt_create_packages=0
			opt_incremental=1
			;;
		--package)
			add_action "package"
			opt_prepare=0
			opt_build=0
			opt_unittest=0
			opt_install=0
			opt_create_packages=1
			opt_incremental=1
			;;
		-j)
			cores=$2
			shift
			;;
		*)
			if [ $# -gt 1 ] ; then
				add_action "$2"
			fi
			;;
	esac
	shift
done

# ensure defaults
if [ ${#opt_actions[*]} -eq 0 ] ; then
	add_action "prepare"
	add_action "make"
	add_action "install"
fi

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


# execute actions
for action_index in ${!opt_actions[*]} ; do
	case ${opt_actions[${action_index}]} in
		*) # TODO: implementation of actions
			;;
	esac
done


if [ ${opt_verbose} -ne 0 ] ; then
	exec_info
fi

if [ ${opt_info} -ne 0 ] ; then
	exit 0
fi

# remove all directories
if [ ${opt_cleanup} -ne 0 ] ; then
	if [ -d ${INSTALL_DIR} ] ; then
		rm -fr ${INSTALL_DIR}
	fi
	if [ -d ${BUILD_DIR} ] ; then
		rm -fr ${BUILD_DIR}
	fi
	if [ -d ${DEPLOY_DIR} ] ; then
		rm -fr ${DEPLOY_DIR}
	fi
	cleanup_index
	exit 0
fi

if [ ${opt_incremental} -eq 0 ] ; then
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

# execution of various commands

if [ ${opt_understand} -ne 0 ] ; then
	exec_prepare
	exec_understand
	exit 0
fi

if [ ${opt_cppcheck} -ne 0 ] ; then
	exec_cppcheck
	exit 0
fi

# default execution

exec_prepare
exec_build
exec_unittest
exec_install
exec_packaging

exit 0
