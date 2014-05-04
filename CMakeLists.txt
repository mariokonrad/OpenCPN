#---------------------------------------------------------------------------
## Author:      Dave Register
## DEB config from: antonm - Anton Martchukov <anton@martchukov.com>
## Update:      sethdart (Jean-Eudes Onfray)
##              with parts from balp (Anders Arnholm)
##***************************************************************************
## *   Copyright (C) 2010 by David S. Register                               *
## *   This program is free software; you can redistribute it and/or modify  *
## *   it under the terms of the GNU General Public License as published by  *
## *   the Free Software Foundation; either version 2 of the License, or     *
## *   (at your option) any later version.                                   *
## *                                                                         *
## *   This program is distributed in the hope that it will be useful,       *
## *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
## *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
## *   GNU General Public License for more details.                          *
## *                                                                         *
## *   You should have received a copy of the GNU General Public License     *
## *   along with this program; if not, write to the                         *
## *   Free Software Foundation, Inc.,                                       *
## *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
## ***************************************************************************

cmake_minimum_required(VERSION 2.8.8)

include(ExternalProject)

#if(COMMAND cmake_policy)
#	CMAKE_POLICY(SET CMP0003 OLD)
#	CMAKE_POLICY(SET CMP0005 OLD)
#	CMAKE_POLICY(SET CMP0011 OLD)
#endif(COMMAND cmake_policy)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Let cmake find additional modules private to opencpn
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

################################################################################
# project information
################################################################################

project(OpenCPN)

SET(VERSION_MAJOR "3")
SET(VERSION_MINOR "3")
SET(VERSION_PATCH "1618")
SET(VERSION_DATE "2014-04-18")

if(APPLE)
	set(PACKAGE_NAME OpenCPN)
else(APPLE)
	set(PACKAGE_NAME opencpn)
endif(APPLE)


#Red Hat:   /etc/redhat-release
#Slackware: /etc/slackware-version
#Slamd64:   /etc/slamd64-version
#Gentoo: /etc/gentoo-release
#Ubuntu: /etc/debian_version


################################################################################
# git information
################################################################################
find_package(Git)
set(GIT_BRANCH "<unknown>")
set(GIT_COMMIT_HASH "<unknown>")
if (GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} log -1 --format=format:%H HEAD
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		OUTPUT_VARIABLE GIT_BRANCH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	message(STATUS "git branch: ${GIT_BRANCH}")
	message(STATUS "git commit hash: ${GIT_COMMIT_HASH}")
endif()

################################################################################
# architecture
################################################################################
IF (NOT WIN32)
	SET (ARCH "i386")
	SET (LIB_INSTALL_DIR "lib")
	IF (EXISTS /etc/debian_version)
		SET (PACKAGE_FORMAT "DEB")
		SET (PACKAGE_DEPS "libc6, libwxgtk2.8-0 (>= 2.8.7.1), libglu1-mesa (>= 7.0.0), libgl1-mesa-glx (>= 7.0.0), zlib1g, bzip2, libportaudio2")
		SET (PACKAGE_RECS "xcalib,xdg-utils")
		SET (LIB_INSTALL_DIR "lib")
		IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
			SET (ARCH "amd64")
			#SET (LIB_INSTALL_DIR "lib64")
		ELSE (CMAKE_SIZEOF_VOID_P MATCHES "8")
			SET (ARCH "i386")
		ENDIF (CMAKE_SIZEOF_VOID_P MATCHES "8")
	ENDIF (EXISTS /etc/debian_version)
	IF (EXISTS /etc/redhat-release)
		SET (PACKAGE_FORMAT "RPM")
		#SET (PACKAGE_DEPS  "wxGTK mesa-libGLU mesa-libGL gettext zlib bzip2 portaudio")
		IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
			SET (ARCH "x86_64")
			SET (LIB_INSTALL_DIR "lib64")
		ELSE (CMAKE_SIZEOF_VOID_P MATCHES "8")
			SET (ARCH "i386")
			SET (LIB_INSTALL_DIR "lib")
		ENDIF (CMAKE_SIZEOF_VOID_P MATCHES "8")
	ENDIF (EXISTS /etc/redhat-release)
	IF (EXISTS /etc/suse-release OR EXISTS /etc/SuSE-release)
		SET (PACKAGE_FORMAT "RPM")
		#SET (PACKAGE_DEPS  "libwx_baseu-2_8-0-wxcontainer MesaGLw libbz2-1 portaudio")
		IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
			SET (ARCH "x86_64")
			SET (LIB_INSTALL_DIR "lib64")
		ELSE (CMAKE_SIZEOF_VOID_P MATCHES "8")
			SET (ARCH "i386")
			SET (LIB_INSTALL_DIR "lib")
		ENDIF (CMAKE_SIZEOF_VOID_P MATCHES "8")
	ENDIF (EXISTS /etc/suse-release OR EXISTS /etc/SuSE-release)
	IF (EXISTS /etc/gentoo-release)
		SET (LIB_INSTALL_DIR "lib${LIB_SUFFIX}")
	ENDIF (EXISTS /etc/gentoo-release)
	IF(APPLE)
		IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
			#IF (CMAKE_OSX_64)
			SET(ARCH "x86_64")
			#ENDIF (CMAKE_OSX_64)
		ENDIF (CMAKE_SIZEOF_VOID_P MATCHES "8")
	ENDIF()
ELSE (NOT WIN32)
	# On WIN32 probably CMAKE_SIZEOF_VOID_P EQUAL 8, but we don't use it at all now...
	SET (ARCH "i386")
ENDIF (NOT WIN32)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
	set (ARCH "armhf")
endif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")

if(ARCH MATCHES "armhf")
	add_definitions(-DARMHF)
endif(ARCH MATCHES "armhf")

message(STATUS "*** Build Architecture is ${ARCH}")

################################################################################
# package version
################################################################################
set(PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

set(PACKAGE_RELEASE "1" CACHE STRING "Package release number")
message(STATUS "*** Staging to build ${PACKAGE_NAME} ${PACKAGE_VERSION} ***")
message(STATUS "")
message(STATUS "*** Staging to build ${PACKAGE_NAME}  ***")

################################################################################
# installation
################################################################################
if(PREFIX)
	set(CMAKE_INSTALL_PREFIX ${PREFIX})
endif(PREFIX)

message(STATUS "*** Will install to ${CMAKE_INSTALL_PREFIX}  ***")
set(PREFIX_BIN bin)
set(PREFIX_INCLUDE include)
set(PREFIX_DATA share)
set(PREFIX_PKGDATA ${PREFIX_DATA}/${PACKAGE_NAME})
set(PREFIX_LIB "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")

################################################################################
# include paths
################################################################################
include_directories("${CMAKE_SOURCE_DIR}/src")

# platform specific include directories

if(WIN32)
	include_directories("${CMAKE_SOURCE_DIR}/src/windows")
endif(WIN32)

if(APPLE)
	include_directories(/usr/X11/include /usr/X11/include/GL)
endif(APPLE)

################################################################################
# compiler options
################################################################################

if(NOT WIN32 AND NOT APPLE)
	add_definitions("-Wall -Wextra -fexceptions -rdynamic")
	add_definitions("-g -fno-strict-aliasing")

	set(C_WARN_FLAGS "-Wall -Wextra")
	#set(CXX_WARN_FLAGS "-Wall -Wextra -Wold-style-cast -Wunreachable-code -Woverloaded-virtual -Wmissing-declarations -Wuninitialized -Winit-self")
	set(CXX_WARN_FLAGS "-Wall -Wextra")

	set(COMMON_DEFINES "-fPIC")
	set(COMMON_DEFINES "${COMMON_DEFINES} -Dnotdef=1") # GDAL

	set(CMAKE_C_FLAGS                  "${COMMON_DEFINES} ${C_WARN_FLAGS} -O2")
	set(CMAKE_C_FLAGS_DEBUG            "${COMMON_DEFINES} ${C_WARN_FLAGS} -O0 -ggdb")
	set(CMAKE_C_FLAGS_MINSIZEREL       "${COMMON_DEFINES} ${C_WARN_FLAGS} -O2")
	set(CMAKE_C_FLAGS_RELEASE          "${COMMON_DEFINES} ${C_WARN_FLAGS} -O3")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO   "${COMMON_DEFINES} ${C_WARN_FLAGS} -O3 -ggdb")

	set(CMAKE_CXX_FLAGS                "${COMMON_DEFINES} ${CXX_WARN_FLAGS} -O2")
	set(CMAKE_CXX_FLAGS_DEBUG          "${COMMON_DEFINES} ${CXX_WARN_FLAGS} -O0 -ggdb")
	set(CMAKE_CXX_FLAGS_MINSIZEREL     "${COMMON_DEFINES} ${CXX_WARN_FLAGS} -O2")
	set(CMAKE_CXX_FLAGS_RELEASE        "${COMMON_DEFINES} ${CXX_WARN_FLAGS} -O3")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${COMMON_DEFINES} ${CXX_WARN_FLAGS} -O3 -ggdb")

	set(CMAKE_EXE_LINKER_FLAGS         "-O2 -Wall -Wextra")
	set(CMAKE_SHARED_LINKER_FLAGS      "-O2 -Wall -Wextra")
	set(CMAKE_MODULE_LINKER_FLAGS      "-O2 -Wall -Wextra")
endif()

if(APPLE)
	add_definitions("-Wall -Wno-unused -fexceptions -Wno-overloaded-virtual")
	add_definitions("-g -fno-strict-aliasing")
	add_definitions("-Wno-deprecated -Wno-deprecated-declarations")

	set(COMMON_DEFINES "${COMMON_DEFINES} -Dnotdef=1") # GDAL

	set(CMAKE_C_FLAGS                  "${COMMON_DEFINES} -O2 -arch ${ARCH}")
	set(CMAKE_C_FLAGS_DEBUG            "${COMMON_DEFINES} -O0 -arch ${ARCH} -g")
	set(CMAKE_C_FLAGS_MINSIZEREL       "${COMMON_DEFINES} -O2 -arch ${ARCH}")
	set(CMAKE_C_FLAGS_RELEASE          "${COMMON_DEFINES} -O3 -arch ${ARCH}")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO   "${COMMON_DEFINES} -O3 -arch ${ARCH} -g")

	set(CMAKE_CXX_FLAGS                "${COMMON_DEFINES} -O2 -arch ${ARCH}")
	set(CMAKE_CXX_FLAGS_DEBUG          "${COMMON_DEFINES} -O0 -arch ${ARCH} -g")
	set(CMAKE_CXX_FLAGS_MINSIZEREL     "${COMMON_DEFINES} -O2 -arch ${ARCH}")
	set(CMAKE_CXX_FLAGS_RELEASE        "${COMMON_DEFINES} -O3 -arch ${ARCH}")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${COMMON_DEFINES} -O3 -arch ${ARCH} -g")

	set(CMAKE_EXE_LINKER_FLAGS         "-O2 -arch ${ARCH}")
	set(CMAKE_SHARED_LINKER_FLAGS      "-O2 -arch ${ARCH}")
	set(CMAKE_MODULE_LINKER_FLAGS      "-O2 -arch ${ARCH}")
ENDIF(APPLE)

add_definitions(-DUSE_GLU_TESS)

if(WIN32)
	add_definitions(-D__MSVC__)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-DPSAPI_VERSION=1)
	add_definitions(-DUSE_PRAGMA_MESSAGE=1)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

	set(COMMON_DEFINES "/nologo")
	set(COMMON_DEFINES "${COMMON_DEFINES} /Dnotdef=1") # GDAL

	# warning suppression
	set(COMMON_DEFINES "${COMMON_DEFINES} /wd4114") # same type qualifier used more than once
	set(COMMON_DEFINES "${COMMON_DEFINES} /wd4284") # This one is to do with "reverse iterator UDT..." Huh?

	# Visual Studio (2012) does not support variadric templates and tries to emulate
	# them up to 5 (default) arguments, but google mock uses them up to 10 arguments.
	if (MSVC11)
		set(COMMON_DEFINES "${COMMON_DEFINES} /D_VARIADIC_MAX=10")
	endif()

	if(MSVC)
		# EH   : exception handling model
		# W1   : warning level 1
		# MD   : multithreaded DLL
		# MDd  : multithreaded DLL (debugging)
		# O1   : optimization: small
		# O2   : optimization: fast
		# Od   : optimization: disabled
		# Ob0  : optimization: inline (disabled)
		# Ob1  : optimization: inline (inline, __inline)
		# Ob2  : optimization: inline (inline, __inline, compiler)
		# D    : define symbol
		# Z1   : complete debugging information
		# RTC1 : runtime error checking

		set(C_WARN_FLAGS   "/W1 /EHsc")
		set(CXX_WARN_FLAGS "/W3 /EHsc")

		set(CMAKE_C_FLAGS                     "${COMMON_DEFINES} ${C_WARN_FLAGS} /MD  /O2  /Ob2")
		set(CMAKE_C_FLAGS_DEBUG               "${COMMON_DEFINES} ${C_WARN_FLAGS} /MDd /Ob0 /Od  /D _DEBUG /Zi /RTC1")
		set(CMAKE_C_FLAGS_MINSIZEREL          "${COMMON_DEFINES} ${C_WARN_FLAGS} /MD  /O1  /Ob1 /D NDEBUG")
		set(CMAKE_C_FLAGS_RELEASE             "${COMMON_DEFINES} ${C_WARN_FLAGS} /MD  /O2  /Ob2 /D _DEBUG")
		set(CMAKE_C_FLAGS_RELWITHDEBINFO      "${COMMON_DEFINES} ${C_WARN_FLAGS} /MD  /O2  /Ob1 /D NDEBUG /Zi")
		set(CMAKE_CXX_FLAGS                   "${COMMON_DEFINES} ${CXX_WARN_FLAGS} /MD  /O2  /Ob2 /D NDEBUG")
		set(CMAKE_CXX_FLAGS_RELEASE           "${COMMON_DEFINES} ${CXX_WARN_FLAGS} /MD  /O2  /Ob2 /D NDEBUG")
		set(CMAKE_CXX_FLAGS_DEBUG             "${COMMON_DEFINES} ${CXX_WARN_FLAGS} /MDd /Ob0 /Od  /D _DEBUG /Zi /RTC1")
		set(CMAKE_CXX_FLAGS_MINSIZEREL        "${COMMON_DEFINES} ${CXX_WARN_FLAGS} /MD  /O1  /Ob1 /D NDEBUG")
		set(CMAKE_CXX_FLAGS_RELEASE           "${COMMON_DEFINES} ${CXX_WARN_FLAGS} /MD  /O2  /Ob2 /D NDEBUG")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO    "${COMMON_DEFINES} ${CXX_WARN_FLAGS} /MD  /O2  /Ob1 /D NDEBUG /Zi")
	endif(MSVC)
endif(WIN32)

message(STATUS "CMAKE_BUILD_TYPE       : ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_COMPILER       : ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_C_FLAGS          : ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_DEBUG    : ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "CMAKE_C_FLAGS_RELEASE  : ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_COMPILER     : ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS        : ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG  : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")

################################################################################
# OpenGL
################################################################################
find_package(OpenGL)
if(OPENGL_GLU_FOUND)
	include_directories(${OPENGL_INCLUDE_DIR})
    add_definitions(-DocpnUSE_GL)
    add_definitions(-DUSE_GLU_TESS)
	message(STATUS "Found OpenGL..." )
	message(STATUS "    Lib: " ${OPENGL_LIBRARIES})
	message(STATUS "    Include: " ${OPENGL_INCLUDE_DIR})
else(OPENGL_GLU_FOUND)
	message(STATUS "OpenGL not found..." )
	set(SKIP_PLUGINS true) # grib_pi contains OpenGL stuff
endif(OPENGL_GLU_FOUND)

################################################################################
# external sources and libraries
################################################################################

include(ExternGmock)
include(ExternZlib)
include(ExternBzip2)
include(ExternPugixml)
include(ExternTinyXML)
include(ExternTcd)
include(ExternWxJson)
# TODO: GDAL

################################################################################
# wxWidgets
################################################################################
set(wxWidgets_USE_LIBS base core net xml html adv aui)

if(OPENGL_GLU_FOUND)
	set(wxWidgets_USE_LIBS ${wxWidgets_USE_LIBS} gl)
endif(OPENGL_GLU_FOUND)

# Find wxWidgets here, and the setting get inherited by all plugins.
# These options can be used to set the linux widgets build type
set(wxWidgets_USE_DEBUG OFF)
set(wxWidgets_USE_UNICODE ON)
set(wxWidgets_USE_UNIVERSAL OFF)
set(wxWidgets_USE_STATIC OFF)

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})
message(STATUS "Found wxWidgets..." )
message(STATUS "wxWidgets Libraries: ${wxWidgets_LIBRARIES}")

################################################################################
# GTK
################################################################################

IF (NOT WIN32 AND NOT APPLE)
	FIND_PACKAGE(GTK2 2.6)
	IF (GTK2_FOUND)
		MESSAGE (STATUS "Found GTK2")
		ADD_DEFINITIONS(-DocpnUSE_GTK_OPTIMIZE)
		ADD_DEFINITIONS(-DocpnHAS_GTK)
		INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
		SET(EXTRA_LIBS ${EXTRA_LIBS} ${GTK2_LIBRARIES})

		# Add a definition to satisfy some linux system builds, avoiding Apple builds
		IF (NOT WIN32 AND NOT APPLE)
			ADD_DEFINITIONS("`pkg-config --cflags --libs gtk+-2.0`")
		ENDIF (NOT WIN32 AND NOT APPLE)
	ELSE(GTK2_FOUND)
		MESSAGE (STATUS "GTK2 Not found...")
	ENDIF (GTK2_FOUND)

	IF(UNIX)
		INCLUDE(FindPortaudio)
		IF (PORTAUDIO_FOUND)
			MESSAGE (STATUS "Portaudio Found")
			INCLUDE_DIRECTORIES(${PORTAUDIO_INCLUDE_DIRS})
			SET(EXTRA_LIBS ${EXTRA_LIBS} ${PORTAUDIO_LIBRARIES})
			ADD_DEFINITIONS(${PORTAUDIO_DEFINITIONS})
			ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
		ELSE (PORTAUDIO_FOUND)
			MESSAGE (STATUS "Portaudio Not Found...")
		ENDIF (PORTAUDIO_FOUND)
		#SET(EXTRA_LIBS ${EXTRA_LIBS} "/usr/lib/i386-linux-gnu/libportaudio.so.2.0.0")
		#ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
	ENDIF(UNIX)
ENDIF (NOT WIN32 AND NOT APPLE)

################################################################################
# gettext
################################################################################

FIND_PACKAGE(Gettext REQUIRED)

################################################################################
# Windows crash reporting
################################################################################

# Enable CrashReporting for Windows
if(WIN32)
	add_definitions(-DOCPN_USE_CRASHRPT)
	include_directories("${CMAKE_SOURCE_DIR}/buildwin/crashrpt")
	set(EXTRA_LIBS ${EXTRA_LIBS} "${CMAKE_SOURCE_DIR}/buildwin/crashrpt/CrashRpt1401.lib")
	set(CMAKE_EXE_LINKER_FLAGS /DEBUG)
endif(WIN32)

################################################################################
# TODO: documentation
################################################################################

SET(SRC_NMEA0183
	src/nmea0183/LatLong.hpp
	src/nmea0183/latlong.cpp
	src/nmea0183/long.cpp
	src/nmea0183/nmea0183.cpp
	src/nmea0183/nmea0183.hpp
	src/nmea0183/Response.hpp
	src/nmea0183/response.cpp
	src/nmea0183/RMB.hpp
	src/nmea0183/rmb.cpp
	src/nmea0183/Sentence.hpp
	src/nmea0183/sentence.cpp
	src/nmea0183/talkerid.cpp
	src/nmea0183/RMC.HPP
	src/nmea0183/rmc.cpp
	src/nmea0183/hexvalue.cpp
	src/nmea0183/lat.cpp
	src/nmea0183/expid.cpp
	src/nmea0183/wpl.hpp
	src/nmea0183/wpl.cpp
	src/nmea0183/rte.hpp
	src/nmea0183/rte.cpp
	src/nmea0183/hdt.hpp
	src/nmea0183/hdt.cpp
	src/nmea0183/hdg.hpp
	src/nmea0183/hdg.cpp
	src/nmea0183/hdm.hpp
	src/nmea0183/hdm.cpp
	src/nmea0183/gll.hpp
	src/nmea0183/gll.cpp
	src/nmea0183/vtg.hpp
	src/nmea0183/vtg.cpp
	src/nmea0183/gga.hpp
	src/nmea0183/gga.cpp
	src/nmea0183/gsv.hpp
	src/nmea0183/gsv.cpp
	src/nmea0183/GPwpl.cpp
	src/nmea0183/GPwpl.hpp
	src/nmea0183/apb.cpp
	src/nmea0183/apb.hpp
	)

add_library(nmea0183 STATIC ${SRC_NMEA0183})
include_directories("${CMAKE_SOURCE_DIR}/src/nmea0183")


SET(HDR_AIS
	src/ais/ais.h
	src/ais/AISTargetQueryDialog.h
	src/ais/AIS_Bitstring.h
	src/ais/AISTargetListDialog.h
	src/ais/AISTargetAlertDialog.h
	src/ais/AIS_Decoder.h
	src/ais/AIS_Target_Data.h
	src/ais/AISTargetTrackPoint.h
	)

SET(SRC_AIS
	src/ais/ais.cpp
	src/ais/AISTargetQueryDialog.cpp
	src/ais/AIS_Bitstring.cpp
	src/ais/AISTargetListDialog.cpp
	src/ais/AISTargetAlertDialog.cpp
	src/ais/AIS_Decoder.cpp
	src/ais/AIS_Target_Data.cpp
	)

SET(SRC_TIDE
	src/tide/Station_Data.cpp
	src/tide/IDX_entry.cpp
	src/tide/TCDataFactory.cpp
	src/tide/TCDS_Ascii_Harmonic.cpp
	src/tide/TCDS_Binary_Harmonic.cpp
	src/tide/TCDataSource.cpp
	)

SET(HDR_TIDE
	src/tide/tide_time.h
	src/tide/Station_Data.h
	src/tide/IDX_entry.h
	src/tide/TC_Error_Code.h
	src/tide/TCDataFactory.h
	src/tide/TCDS_Ascii_Harmonic.h
	src/tide/TCDS_Binary_Harmonic.h
	src/tide/TCDataSource.h
	)

SET(SRC_PLUGIN
	src/plugin/PlugInManager.cpp
	src/plugin/PlugInContainer.cpp
	src/plugin/PlugInToolbarToolContainer.cpp
	src/plugin/PluginPanel.cpp
	src/plugin/PluginListPanel.cpp
	src/plugin/OCPN_MsgEvent.cpp
	src/plugin/ChartPlugInWrapper.cpp
	)

SET(HDR_PLUGIN
	src/plugin/PlugInManager.h
	src/plugin/PlugInContainer.h
	src/plugin/PlugInToolbarToolContainer.h
	src/plugin/PluginPanel.h
	src/plugin/PluginListPanel.h
	src/plugin/OCPN_MsgEvent.h
	src/plugin/ocpn_plugin.h
	src/plugin/ChartPlugInWrapper.h
	)

SET(SRC_SOUND
	src/sound/OCPNSoundData.cpp
	src/sound/OCPN_Sound.cpp
	)

SET(HDR_SOUND
	src/sound/OCPNSoundData.h
	src/sound/OCPN_Sound.h
	)

add_library(global STATIC
	src/global/OCPN.cpp
	src/global/OCPN_GUI.cpp
	src/global/OCPN_Navigation.cpp
	src/global/OCPN_AIS.cpp
	src/global/OCPN_WatchDog.cpp
	src/global/OCPN_System.cpp
	src/global/OCPN_Runtime.cpp

	src/global/ColorScheme.h
	src/global/ColorManager.h
	src/global/ColorProvider.h
	src/global/GUI.h
	src/global/Navigation.h
	src/global/WatchDog.h
	src/global/System.h
	src/global/Runtime.h
	src/global/OCPN.h
	src/global/OCPN_GUI.h
	src/global/OCPN_Navigation.h
	src/global/OCPN_AIS.h
	src/global/OCPN_WatchDog.h
	src/global/OCPN_System.h
	src/global/OCPN_Runtime.h
	)

add_library(geo STATIC
	src/geo/Polygon.cpp
	src/geo/triangulate.cpp
	src/geo/lmfit.cpp
	src/geo/LineClip.cpp
	src/geo/Position.cpp
	src/geo/BoundingBox.cpp
	src/geo/LatLonBoundingBox.cpp
	src/geo/GeoRef.cpp
	src/geo/Geodesic.cpp

	src/geo/Polygon.h
	src/geo/triangulate.h
	src/geo/lmfit.h
	src/geo/LineClip.h
	src/geo/Position.h
	src/geo/BoundingBox.h
	src/geo/LatLonBoundingBox.h
	src/geo/GeoRef.h
	src/geo/Geodesic.h
	)

add_library(util STATIC
	src/util/Vector2D.cpp
	src/util/Vector2D.h
	src/util/ValueFilter.cpp
	src/util/ValueFilter.h
	src/util/crc32.cpp
	src/util/crc32.h
	src/util/math.h
	src/util/uuid.cpp
	src/util/uuid.h
	)

SET(SRC_CHART
	src/chart/gshhs/Projection.cpp
	src/chart/gshhs/GshhsPolygon.cpp
	src/chart/gshhs/GSHHSChart.cpp
	src/chart/gshhs/GshhsPolyCell.cpp
	src/chart/gshhs/GshhsPolyReader.cpp
	src/chart/gshhs/GshhsReader.cpp
	src/chart/ChartDummy.cpp
	src/chart/ChartDB.cpp
	src/chart/ChartDatabase.cpp
	src/chart/ChartTableHeader.cpp
	src/chart/ChartTableEntry.cpp
	src/chart/ChartClassDescriptor.cpp
	src/chart/ChartGEO.cpp
	src/chart/ChartKAP.cpp
	src/chart/ChartStack.cpp
	src/chart/ChartBase.cpp
	src/chart/ChartBaseBSB.cpp
	src/chart/ChartSymbols.cpp
	src/chart/S57RegistrarMgr.cpp
	src/chart/RazdsParser.cpp
	)

SET(HDR_CHART
	src/chart/gshhs/QLineF.h
	src/chart/gshhs/Projection.h
	src/chart/gshhs/PolygonFileHeader.h
	src/chart/gshhs/GshhsPoint.h
	src/chart/gshhs/GshhsPolygon.h
	src/chart/gshhs/GSHHSChart.h
	src/chart/gshhs/GshhsPolyCell.h
	src/chart/gshhs/GshhsPolyReader.h
	src/chart/gshhs/GshhsReader.h
	src/chart/Refpoint.h
	src/chart/ChartTableHeader.h
	src/chart/ChartTableEntry.h
	src/chart/ChartClassDescriptor.h
	src/chart/ChartDB.h
	src/chart/CacheEntry.h
	src/chart/ChartDatabase.h
	src/chart/ChartGroupElement.h
	src/chart/ChartGroup.h
	src/chart/ChartGEO.h
	src/chart/ChartKAP.h
	src/chart/ChartStack.h
	src/chart/ChartType.h
	src/chart/ChartFamily.h
	src/chart/S57Sector.h
	src/chart/ChartBase.h
	src/chart/PlyPoint.h
	src/chart/ChartDummy.h
	src/chart/ChartBaseBSB.h
	src/chart/ChartSymbols.h
	src/chart/SymbolSizeInfo.h
	src/chart/ChartSymbol.h
	src/chart/LineStyle.h
	src/chart/OCPNPattern.h
	src/chart/Lookup.h
	src/chart/S57ClassRegistrar.h
	src/chart/S57RegistrarMgr.h
	src/chart/S57Light.h
	src/chart/RazdsParser.h
	)

SET(HDRS
	${HDR_AIS}
	${HDR_TIDE}
	${HDR_PLUGIN}
	${HDR_SOUND}
	${HDR_CHART}

	src/gpx/gpx.h
	src/gpx/ParseGPXDateTime.h

	src/tide/TideCurrentManager.h
	src/tide/TCMgr.h

	src/garmin/cpo_satellite_data.h
	src/garmin/unit_info.h
	src/garmin/usb_packet.h
	src/garmin/GARMIN_Serial_Thread.h
	src/garmin/GARMIN_USB_Thread.h
	src/garmin/GarminProtocolHandler.h

	src/print/Table.h
	src/print/PrintTable.h
	src/print/PrintCell.h

	src/navigation/AnchorDist.h
	src/navigation/MagneticVariation.h
	src/navigation/RouteTracker.h
	src/navigation/DefaultRouteTracker.h
	src/navigation/RouteManager.h
	src/navigation/WaypointManager.h

	src/gui/Style.h
	src/gui/Tool.h
	src/gui/Icon.h
	src/gui/StyleManager.h
	src/gui/DefaultStyleManager.h
	src/gui/StyleReader.h
	src/gui/XMLStyleReader.h
	src/gui/FontDesc.h
	src/gui/FontManager.h

	src/graphics/MergeBitmaps.h
	src/graphics/OCPNBitmap.h
	src/graphics/ocpn_pixel.h
	src/graphics/OCPNMemDC.h

	src/OCPN_Version.h
	src/Units.h
	src/DataStream.h
	src/Config.h
	src/LogMessageOnce.h
	src/DimeControl.h
	src/ChartCanvas.h
	src/MemoryStatus.h
	src/MainFrame.h
	src/SystemTime.h
	src/GUI_IDs.h
	src/SerialPorts.h
	src/decl_exp.h
	src/ocpnDC.h
	src/App.h
	src/UserColors.h
	src/StatusBar.h
	src/TimedPopupWin.h
	src/AnnunText.h
	src/CDI.h
	src/MyPrintout.h
	src/timers.h
	src/ThumbData.h
	src/EmbossData.h
	src/OCPNListCtrl.h
	src/CM93DSlide.h
	src/GoToPositionDialog.h
	src/RolloverWin.h
	src/S57QueryDialog.h
	src/MarkIcon.h
	src/SendToGpsDlg.h
	src/TCWin.h
	src/ChInfoWin.h
	src/Quilt.h
	src/QuiltPatch.h
	src/QuiltCandidate.h
	src/Hyperlink.h
	src/NavObjectCollection.h
	src/NavObjectChanges.h
	src/RoutePoint.h
	src/Route.h
	src/SelectItem.h
	src/Select.h
	src/FontMgr.h
	src/NMEALogWindow.h
	src/WindowDestroyListener.h
	src/TTYWindow.h
	src/TTYScroll.h
	src/Layer.h
	src/ConsoleCanvas.h
	src/ViewPort.h
	src/Track.h
	src/Routeman.h
	src/WayPointman.h
	src/RouteManagerDialog.h
	src/RouteProp.h
	src/Daylight.h
	src/MarkInfo.h
	src/LatLonTextCtrl.h
	src/RoutePrintout.h
	src/PositionConvert.h
	src/PositionParser.h
	src/RoutePrintSelection.h
	src/StatWin.h
	src/TextStatWin.h
	src/PianoWin.h
	src/ThumbWin.h
	src/OptionDialog.h
	src/OptionIDs.h
	src/ChartGroupsUI.h
	src/SentenceListDlg.h
	src/Kml.h
	src/KmlFormatDialog.h
	src/Undo.h
	src/UndoAction.h
	src/AboutDialog.h
	src/ProjectionType.h
	src/ToolBarSimple.h
	src/ToolTipWin.h
	src/ToolBarTool.h
	src/ToolbarMOBDialog.h
	src/GrabberWin.h
	src/OCPNFloatingToolbarDialog.h
	src/FloatingCompassWindow.h
	src/OCPN_DataStreamEvent.h
	src/ConnectionParams.h
	src/OCP_DataStreamInput_Thread.h
	src/dsPortType.h
	src/Multiplexer.h
	src/OCPNRegion.h
	src/OCPNRegionIterator.h
	src/TrackPropDlg.h
	src/OCPNTrackListCtrl.h
	src/LinkPropDlg.h
	src/OCPN_AlertDialog.h
	src/NMEA_Msg_Container.h
	src/MessageBox.h
	src/ChartDirectoryInfo.h
	)

SET(SRCS
	${SRC_AIS}
	${SRC_TIDE}
	${SRC_PLUGIN}
	${SRC_SOUND}
	${SRC_CHART}

	src/gpx/ParseGPXDateTime.cpp

	src/tide/TCMgr.cpp

	src/print/Table.cpp
	src/print/PrintTable.cpp
	src/print/PrintCell.cpp

	src/navigation/AnchorDist.cpp
	src/navigation/MagneticVariation.cpp
	src/navigation/DefaultRouteTracker.cpp

	src/garmin/GARMIN_Serial_Thread.cpp
	src/garmin/GARMIN_USB_Thread.cpp
	src/garmin/GarminProtocolHandler.cpp

	src/gui/Style.cpp
	src/gui/Tool.cpp
	src/gui/Icon.cpp
	src/gui/DefaultStyleManager.cpp
	src/gui/XMLStyleReader.cpp
	src/gui/FontDesc.cpp

	src/graphics/MergeBitmaps.cpp
	src/graphics/OCPNBitmap.cpp
	src/graphics/ocpn_pixel.cpp
	src/graphics/OCPNMemDC.cpp

	src/OCPN_Version.cpp
	src/MemoryStatus.cpp
	src/SerialPorts.cpp
	src/MainFrame.cpp
	src/SystemTime.cpp
	src/MessageBox.cpp
	src/TimedPopupWin.cpp
	src/AnnunText.cpp
	src/CDI.cpp
	src/MyPrintout.cpp
	src/ocpnDC.cpp
	src/DimeControl.cpp
	src/ChartCanvas.cpp
	src/App.cpp
	src/UserColors.cpp
	src/TCWin.cpp
	src/S57QueryDialog.cpp
	src/GoToPositionDialog.cpp
	src/CM93DSlide.cpp
	src/RolloverWin.cpp
	src/ChInfoWin.cpp
	src/OCPNListCtrl.cpp
	src/Quilt.cpp
	src/EmbossData.cpp
	src/ThumbData.cpp
	src/QuiltPatch.cpp
	src/QuiltCandidate.cpp
	src/Hyperlink.cpp
	src/NavObjectCollection.cpp
	src/NavObjectChanges.cpp
	src/RoutePoint.cpp
	src/Route.cpp
	src/SelectItem.cpp
	src/Select.cpp
	src/FontMgr.cpp
	src/NMEALogWindow.cpp
	src/TTYWindow.cpp
	src/TTYScroll.cpp
	src/SendToGpsDlg.cpp
	src/Layer.cpp
	src/ConsoleCanvas.cpp
	src/ViewPort.cpp
	src/Units.cpp
	src/Config.cpp
	src/LogMessageOnce.cpp
	src/Track.cpp
	src/Routeman.cpp
	src/WayPointman.cpp
	src/RouteManagerDialog.cpp
	src/Daylight.cpp
	src/RouteProp.cpp
	src/MarkInfo.cpp
	src/LatLonTextCtrl.cpp
	src/RoutePrintout.cpp
	src/RoutePrintSelection.cpp
	src/PositionConvert.cpp
	src/PositionParser.cpp
	src/StatWin.cpp
	src/TextStatWin.cpp
	src/PianoWin.cpp
	src/ThumbWin.cpp
	src/OptionDialog.cpp
	src/ChartGroupsUI.cpp
	src/SentenceListDlg.cpp
	src/Kml.cpp
	src/KmlFormatDialog.cpp
	src/Undo.cpp
	src/UndoAction.cpp
	src/AboutDialog.cpp
	src/ToolBarSimple.cpp
	src/ToolTipWin.cpp
	src/ToolBarTool.cpp
	src/ToolbarMOBDialog.cpp
	src/GrabberWin.cpp
	src/OCPNFloatingToolbarDialog.cpp
	src/FloatingCompassWindow.cpp
	src/DataStream.cpp
	src/OCPN_DataStreamEvent.cpp
	src/ConnectionParams.cpp
	src/OCP_DataStreamInput_Thread.cpp
	src/Multiplexer.cpp
	src/OCPNRegion.cpp
	src/OCPNRegionIterator.cpp
	src/TrackPropDlg.cpp
	src/OCPNTrackListCtrl.cpp
	src/LinkPropDlg.cpp
	src/OCPN_AlertDialog.cpp
	)

set(PACKAGE_HEADERS "")

if(OPENGL_GLU_FOUND)
	set(HDRS ${HDRS}
		src/glChartCanvas.h
		src/glTextureDescriptor.h
	   )
	set(SRCS ${SRCS}
		src/glChartCanvas.cpp
		src/glTextureDescriptor.cpp
	   )
endif(OPENGL_GLU_FOUND)


option (USE_S57 "Enable S57 ENC support" ON)
if (USE_S57)
	message (STATUS "S57 ENC support: enabled")
	add_definitions(-DUSE_S57)

	set(SRC_S57ENC
		src/TexFont.h
		src/TexFont.cpp

		src/chart/geometry/ExtendedGeometry.cpp
		src/chart/geometry/ExtendedGeometry.h
		src/chart/geometry/TriPrim.cpp
		src/chart/geometry/TriPrim.h
		src/chart/geometry/PolyTriGroup.cpp
		src/chart/geometry/PolyTriGroup.h
		src/chart/geometry/PolyTrapGroup.cpp
		src/chart/geometry/PolyTrapGroup.h
		src/chart/geometry/PolyTessGeo.cpp
		src/chart/geometry/PolyTessGeo.h
		src/chart/geometry/PolyTessGeoTrap.cpp
		src/chart/geometry/PolyTessGeoTrap.h

		src/chart/S52Color.h
		src/chart/ColorTable.h
		src/chart/RenderFromHPGL.h
		src/chart/RenderFromHPGL.cpp
		src/chart/s52cnsy.cpp
		src/chart/s52s57.h
		src/chart/S52ColorProvider.h
		src/chart/S52ColorProvider.cpp
		src/chart/s52plib.h
		src/chart/s52plib.cpp
		src/chart/s52utils.cpp
		src/chart/S57Chart.cpp
		src/chart/s52utils.h
		src/chart/S57Chart.h
		src/chart/M_COVR_Desc.cpp
		src/chart/OCPNOffsetListCtrl.cpp
		src/chart/COVR_Set.cpp
		src/chart/CM93_attr_block.cpp
		src/chart/CM93OffsetDialog.cpp
		src/chart/CM93Dictionary.cpp
		src/chart/CM93Chart.cpp
		src/chart/CM93Manager.cpp
		src/chart/CM93compchart.cpp
		src/chart/OCPNOffsetListCtrl.h
		src/chart/CM93OffsetDialog.h
		src/chart/M_COVR_Desc.h
		src/chart/COVR_Set.h
		src/chart/CM93compchart.h
		src/chart/FindCM93Dictionary.h
		src/chart/CM93Dictionary.h
		src/chart/CM93_attr_block.h
		src/chart/CM93Manager.h
		src/chart/CM93Chart.h
		src/chart/CM93Geometry.h

		src/myiso8211/ddffielddefn.cpp
		src/myiso8211/ddfmodule.cpp
		src/myiso8211/ddfrecord.cpp
		src/myiso8211/ddfsubfielddefn.cpp
		src/myiso8211/ddffield.cpp
		src/myiso8211/ddfutils.cpp

		src/mygdal/cpl_config.h
		src/mygdal/gdal_priv.h
		src/mygdal/cpl_conv.h
		src/mygdal/iso8211.h
		src/mygdal/cpl_csv.h
		src/mygdal/ogr_api.h
		src/mygdal/cpl_error.h
		src/mygdal/ogr_attrind.h
		src/mygdal/cpl_list.h
		src/mygdal/cpl_minixml.h
		src/mygdal/ogr_feature.h
		src/mygdal/cpl_multiproc.h
		src/mygdal/ogr_geometry.h
		src/mygdal/cpl_odbc.h
		src/mygdal/ogr_p.h
		src/mygdal/ogr_core.h
		src/mygdal/cpl_port.h
		src/mygdal/ogr_s57.h
		src/mygdal/cpl_string.h
		src/mygdal/ogr_spatialref.h
		src/mygdal/cpl_vsi.h
		src/mygdal/ogrsf_frmts.h
		src/mygdal/gdal.h
		src/mygdal/s57.h
		src/mygdal/gdal_frmts.h
		src/mygdal/ddfrecordindex.cpp
		src/mygdal/cpl_conv.cpp
		src/mygdal/cpl_csv.cpp
		src/mygdal/cpl_error.cpp
		src/mygdal/cpl_findfile.cpp
		src/mygdal/cpl_minixml.cpp
		src/mygdal/cpl_path.cpp
		src/mygdal/cpl_string.cpp
		src/mygdal/cpl_vsisimple.cpp
		src/mygdal/cplgetsymbol.cpp
		src/mygdal/ograssemblepolygon.cpp
		src/mygdal/ogrcurve.cpp
		src/mygdal/ogrfeature.cpp
		src/mygdal/ogrfeaturedefn.cpp
		src/mygdal/ogrfielddefn.cpp
		src/mygdal/ogrgeometry.cpp
		src/mygdal/ogrgeometrycollection.cpp
		src/mygdal/ogrgeometryfactory.cpp
		src/mygdal/ogrlinearring.cpp
		src/mygdal/ogrlinestring.cpp
		src/mygdal/ogrmultilinestring.cpp
		src/mygdal/ogrmultipoint.cpp
		src/mygdal/ogrmultipolygon.cpp
		src/mygdal/ogrpoint.cpp
		src/mygdal/ogrpolygon.cpp
		src/mygdal/ogrs57datasource.cpp
		src/mygdal/ogrlayer.cpp
		src/mygdal/ogrs57layer.cpp
		src/mygdal/ogrutils.cpp
		src/mygdal/s57classregistrar.cpp
		src/mygdal/s57featuredefns.cpp
		src/mygdal/s57reader.cpp
		src/mygdal/gdal_misc.cpp
		)

	add_library(S57ENC STATIC ${SRC_S57ENC})
	add_dependencies(S57ENC tinyxml wxjson)
	include_directories("${CMAKE_SOURCE_DIR}/src/mygdal")
	set(EXTRA_LIBS ${EXTRA_LIBS} S57ENC)
else (USE_S57)
	message (STATUS "S57 ENC support: disabled")
endif (USE_S57)

option (USE_GARMINHOST "Enable Garmin Host Mode support" ON)
if (USE_GARMINHOST)
	message (STATUS "Garmin Host Mode support: enabled")
	add_definitions(-DUSE_GARMINHOST)

	set(SRC_GARMINHOST
		src/garmin/jeeps/gpsapp.c
		src/garmin/jeeps/gpscom.c
		src/garmin/jeeps/gpsmath.c
		src/garmin/jeeps/gpsmem.c
		src/garmin/jeeps/gpsprot.c
		src/garmin/jeeps/gpsread.c
		src/garmin/jeeps/gpsdevice.c
		src/garmin/jeeps/gpsdevice_ser.c
		src/garmin/jeeps/gpsdevice_usb.c
		src/garmin/jeeps/gpsrqst.c
		src/garmin/jeeps/gpssend.c
		src/garmin/jeeps/gpsserial.c
		src/garmin/jeeps/gpsutil.c
		src/garmin/jeeps/gpsusbread.c
		src/garmin/jeeps/gpsusbsend.c
		src/garmin/jeeps/gpsusbcommon.c
		src/garmin/jeeps/garmin_wrapper_utils.c
		src/garmin/jeeps/garmin_wrapper.cpp
		src/garmin/jeeps/garmin_gps.h
		src/garmin/jeeps/garmin_wrapper.h
		src/garmin/jeeps/gpsapp.h
		src/garmin/jeeps/gpsdatum.h
		src/garmin/jeeps/gpsfmt.h
		src/garmin/jeeps/gpsmath.h
		src/garmin/jeeps/gpsport.h
		src/garmin/jeeps/gpsprot.h
		src/garmin/jeeps/gpsrqst.h
		src/garmin/jeeps/gpsserial.h
		src/garmin/jeeps/gpsusbint.h
		src/garmin/jeeps/garminusb.h
		src/garmin/jeeps/garmin_wrapper_utils.h
		src/garmin/jeeps/gpscom.h
		src/garmin/jeeps/gpsdevice.h
		src/garmin/jeeps/gpsinput.h
		src/garmin/jeeps/gpsmem.h
		src/garmin/jeeps/gpsproj.h
		src/garmin/jeeps/gpsread.h
		src/garmin/jeeps/gpssend.h
		src/garmin/jeeps/gpsusbcommon.h
		src/garmin/jeeps/gpsutil.h
		)

	if(WIN32)
		SET(SRC_GARMINHOST ${SRC_GARMINHOST} src/garmin/jeeps/gpsusbwin.c)
	endif(WIN32)

	add_library(GARMINHOST STATIC ${SRC_GARMINHOST})
	include_directories("${CMAKE_SOURCE_DIR}/src/garmin/jeeps")
	set(EXTRA_LIBS ${EXTRA_LIBS} GARMINHOST)

	if(WIN32)
		add_definitions(-D__WIN32__)
	endif(WIN32)
else (USE_GARMINHOST)
	message (STATUS "Garmin Host Mode support: disabled")
endif (USE_GARMINHOST)


# If we build for windows systems, we also include the resource file
# containing the manifest, icon and other resources
if(WIN32)
	set(SRCS ${SRCS}
		src/windows/compatibility.cpp
		src/windows/compatibility.h
		src/windows/WinConsole.cpp
		src/windows/WinConsole.h
		${CMAKE_CURRENT_BINARY_DIR}/src/opencpn.rc
		)
endif(WIN32)

if(UNIX)
	set(SRCS ${SRCS}
		src/linux/X11FontPicker.cpp
		src/linux/X11FontPicker.h
		)
endif()

# Various data files
set(gshhs
	data/gshhs/poly-c-1.dat
	data/gshhs/poly-i-1.dat
	data/gshhs/poly-l-1.dat
	data/gshhs/wdb_borders_c.b
	data/gshhs/wdb_borders_i.b
	data/gshhs/wdb_borders_l.b
	data/gshhs/wdb_rivers_c.b
	data/gshhs/wdb_rivers_i.b
	data/gshhs/wdb_rivers_l.b
	)

set(uiData
	src/bitmaps/styles.xml
	src/bitmaps/toolicons_journeyman.png
	src/bitmaps/toolicons_journeyman_flat.png
	src/bitmaps/toolicons_traditional.png
	)

if (USE_S57)
	set(s57data
		data/s57data/attdecode.csv
		data/s57data/S52RAZDS.RLE
		data/s57data/s57attributes.csv
		data/s57data/s57expectedinput.csv
		data/s57data/s57objectclasses.csv
		data/s57data/rastersymbols-dark.png
		data/s57data/rastersymbols-day.png
		data/s57data/rastersymbols-dusk.png
		data/s57data/chartsymbols.xml
		)
endif (USE_S57)

set(tcData
	data/tcdata/HARMONIC
	data/tcdata/HARMONIC.IDX
	data/tcdata/README.harmonics
	)

if(APPLE)
	set(SRCS ${SRCS}
		src/osx/macutils.h
		src/osx/macutils.c
		)
ENDIF(APPLE)

#  Mac has trouble finding libgobject-2.0.dylib
#  We look for it explicitely
if(xAPPLE)
	# Assume pkg-config is available.
	include( FindPkgConfig )
	PKG_SEARCH_MODULE( GOBJECT REQUIRED gobject-2.0 )
	set(EXTRA_LIBS ${EXTRA_LIBS} ${GOBJECT_LDFLAGS})
	set(EXTRA_LIBS ${EXTRA_LIBS} "iconv")
endif(xAPPLE)

if(WIN32)
	add_executable(${PACKAGE_NAME} WIN32 ${HDRS} ${SRCS})
endif(WIN32)


if(APPLE)
	set_source_files_properties(${gshhs} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/gshhs)
	set_source_files_properties(${uiData} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/uidata)
	set_source_files_properties(${s57data} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/s57data)
	set_source_files_properties(${tcData} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/tcdata)
	set_source_files_properties(data/license.txt PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport)

	FILE(GLOB SOUND_FILES "${CMAKE_SOURCE_DIR}/data/sounds/*")
	FOREACH (_currentSoundFile ${SOUND_FILES})
		SET_SOURCE_FILES_PROPERTIES(
			${_currentSoundFile} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/sounds )
	ENDFOREACH (_currentSoundFile )

	SET_SOURCE_FILES_PROPERTIES(data/doc/help_web.html PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/doc)
	SET_SOURCE_FILES_PROPERTIES(data/doc/help_en_US.html PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/doc)

	FILE(GLOB DOC_FILES "${CMAKE_SOURCE_DIR}/data/doc/images/*")
	FOREACH (_currentDocFile ${DOC_FILES})
		SET_SOURCE_FILES_PROPERTIES(
		${_currentDocFile}
		PROPERTIES MACOSX_PACKAGE_LOCATION "SharedSupport/doc/images")
	ENDFOREACH (_currentDocFile)

	SET_SOURCE_FILES_PROPERTIES(buildosx/opencpn.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
	SET_SOURCE_FILES_PROPERTIES(buildosx/MainMenu.xib PROPERTIES MACOSX_PACKAGE_LOCATION Resources/English.lproj)
	# These variables get substituted into the Info.plist template file at build time
	SET(MACOSX_BUNDLE_ICON_FILE opencpn.icns)
	SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PACKAGE_VERSION}")
	SET(MACOSX_BUNDLE_BUNDLE_NAME "OpenCPN")
	SET(MACOSX_BUNDLE_GUI_IDENTIFIER "org.opencpn")

	ADD_EXECUTABLE(${PACKAGE_NAME} MACOSX_BUNDLE
		${HDRS}
		${SRCS}
		${gshhs}
		${uiData}
		${s57data}
		${tcData}
		data/license.txt
		${SOUND_FILES}
		data/doc/help_web.html
		data/doc/help_en_US.html
		${DOC_FILES}
		buildosx/opencpn.icns
		buildosx/MainMenu.xib
		)

	INSTALL(TARGETS
		${PACKAGE_NAME}
		BUNDLE DESTINATION ${PREFIX_BIN}
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

	target_link_libraries(${PACKAGE_NAME}
		${wxWidgets_LIBRARIES}
		nmea0183
		${EXTRA_LIBS}
		global
		geo
		util
		)
	set_target_properties(${PACKAGE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST
		"${CMAKE_SOURCE_DIR}/buildosx/Info.plist.in")
ENDIF(APPLE)

IF(UNIX AND NOT APPLE)
	ADD_EXECUTABLE(${PACKAGE_NAME} ${HDRS} ${SRCS})
ENDIF(UNIX AND NOT APPLE)


If(NOT APPLE)
	target_link_libraries(${PACKAGE_NAME}
		pugixml
		tinyxml
		tcd
		wxjson
		${wxWidgets_LIBRARIES}
		nmea0183
		${EXTRA_LIBS}
		global
		geo
		util
		)
ENDIF(NOT APPLE)

IF(WIN32)
	target_link_libraries(${PACKAGE_NAME}
		setupapi.lib
		gdiplus.lib
		glu32.lib
		Psapi.lib
		wxjson
		)
ENDIF(WIN32)

IF(UNIX)
	target_link_libraries(${PACKAGE_NAME}
		${OPENGL_LIBRARIES}
		${GETTEXT_LIBRARY}
		bzip2
		zlib
		)
ENDIF(UNIX)

# Certain older Cmake FindGTK2 modules ( e.g. cmake-2.8.0-2) do not yield all of the required link libraries
# So, add them manually.  These declarations may be redundant in some architectures, but do no harm.
IF(UNIX)
	target_link_libraries(${PACKAGE_NAME} dl)
ENDIF(UNIX)

IF(GTK2_FOUND)
	target_link_libraries(${PACKAGE_NAME} gobject-2.0)
ENDIF(GTK2_FOUND)


IF(NOT APPLE)
	IF(WIN32)
		INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION ".")
	ENDIF(WIN32)

	IF(UNIX AND NOT APPLE)
		INSTALL(TARGETS ${PACKAGE_NAME}
			RUNTIME DESTINATION
				${PREFIX_BIN}
			PERMISSIONS
				OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
	ENDIF(UNIX AND NOT APPLE)

	IF(WIN32)
		SET(PREFIX_PKGDATA ".")
	ENDIF(WIN32)

	INSTALL(FILES ${gshhs}
		DESTINATION
			${PREFIX_PKGDATA}/gshhs
		PERMISSIONS
			OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	INSTALL(FILES ${uiData}
		DESTINATION
			${PREFIX_PKGDATA}/uidata
		PERMISSIONS
			OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

	IF (USE_S57)
		INSTALL(FILES ${s57data} DESTINATION ${PREFIX_PKGDATA}/s57data PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	ENDIF (USE_S57)

	INSTALL(FILES ${tcData} DESTINATION ${PREFIX_PKGDATA}/tcdata PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	INSTALL(FILES data/license.txt DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	INSTALL(FILES data/copyright DESTINATION ${PREFIX_DATA}/doc/${PACKAGE_NAME} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	INSTALL(FILES data/changelog.Debian.gz DESTINATION ${PREFIX_DATA}/doc/${PACKAGE_NAME} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	INSTALL(DIRECTORY data/sounds/ DESTINATION ${PREFIX_PKGDATA}/sounds )
	INSTALL(DIRECTORY data/doc/ DESTINATION ${PREFIX_PKGDATA}/doc )

	IF(UNIX)
		INSTALL(FILES data/opencpn.png DESTINATION ${PREFIX_DATA}/icons/hicolor/48x48/apps PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
		INSTALL(FILES src/bitmaps/other_svg_src/opencpn.svg DESTINATION ${PREFIX_DATA}/icons/hicolor/scalable/apps PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
		INSTALL(FILES data/opencpn.desktop DESTINATION ${PREFIX_DATA}/applications PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ )
	ENDIF(UNIX)
ENDIF(NOT APPLE)


FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
IF (GETTEXT_XGETTEXT_EXECUTABLE)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot.dummy
		COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} --force-po --package-name=${PACKAGE_NAME} --package-version="${PACKAGE_VERSION}" --output=${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot  --keyword=_ --width=80 --files-from=${CMAKE_SOURCE_DIR}/po/POTFILES.in
		DEPENDS ${CMAKE_SOURCE_DIR}/po/POTFILES.in ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "pot-update [${PACKAGE_NAME}]: Generated pot file."
		)
	ADD_CUSTOM_TARGET(pot-update COMMENT "pot-update: Done." DEPENDS ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot.dummy)
ENDIF(GETTEXT_XGETTEXT_EXECUTABLE)

MACRO(GETTEXT_UPDATE_PO _potFile)
	SET(_poFiles ${_potFile})
	GET_FILENAME_COMPONENT(_absPotFile ${_potFile} ABSOLUTE)
	FOREACH (_currentPoFile ${ARGN})
		GET_FILENAME_COMPONENT(_absFile ${_currentPoFile} ABSOLUTE)
		GET_FILENAME_COMPONENT(_poBasename ${_absFile} NAME_WE)
		ADD_CUSTOM_COMMAND(
			OUTPUT ${_absFile}.dummy
			COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --width=80 --strict --quiet --update --backup=none --no-location -s ${_absFile} ${_absPotFile}
			DEPENDS ${_absPotFile} ${_absFile}
			COMMENT "po-update [${_poBasename}]: Updated po file."
			)
		SET(_poFiles ${_poFiles} ${_absFile}.dummy)
	ENDFOREACH (_currentPoFile )
	ADD_CUSTOM_TARGET(po-update COMMENT "po-update: Done." DEPENDS ${_poFiles})
ENDMACRO(GETTEXT_UPDATE_PO)

IF (GETTEXT_MSGMERGE_EXECUTABLE)
	FILE(GLOB PACKAGE_PO_FILES ${CMAKE_SOURCE_DIR}/po/*.po)
	GETTEXT_UPDATE_PO(${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot ${PACKAGE_PO_FILES})
ENDIF(GETTEXT_MSGMERGE_EXECUTABLE)

SET(_gmoFiles)

MACRO(GETTEXT_BUILD_MO _poFile _lang)
	GET_FILENAME_COMPONENT(_absFile ${_poFile} ABSOLUTE)
	GET_FILENAME_COMPONENT(_poBasename ${_absFile} NAME_WE)
	SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_poBasename}.mo)

	ADD_CUSTOM_COMMAND(
		OUTPUT ${_gmoFile}
		COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_absFile}
		DEPENDS ${_absFile}
		COMMENT "i18n [${_poBasename}]: Created mo file."
	)

	IF(APPLE)
		INSTALL(FILES ${_gmoFile} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Resources/${_lang}.lproj RENAME ${PACKAGE_NAME}.mo )
	ELSE(APPLE)
		INSTALL(FILES ${_gmoFile} DESTINATION ${PREFIX_DATA}/locale/${_lang}/LC_MESSAGES RENAME ${PACKAGE_NAME}.mo PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	ENDIF(APPLE)

	SET(_gmoFiles ${_gmoFiles} ${_gmoFile})
ENDMACRO(GETTEXT_BUILD_MO)

if(GETTEXT_MSGFMT_EXECUTABLE)
	GETTEXT_BUILD_MO(po/opencpn_cs_CZ.po cs)
	GETTEXT_BUILD_MO(po/opencpn_da_DK.po da)
	GETTEXT_BUILD_MO(po/opencpn_de_DE.po de)
	GETTEXT_BUILD_MO(po/opencpn_es_ES.po es)
	GETTEXT_BUILD_MO(po/opencpn_fr_FR.po fr)
	GETTEXT_BUILD_MO(po/opencpn_it_IT.po it)
	GETTEXT_BUILD_MO(po/opencpn_nl_NL.po nl)
	GETTEXT_BUILD_MO(po/opencpn_pl_PL.po pl)
	GETTEXT_BUILD_MO(po/opencpn_ru_RU.po ru)
	GETTEXT_BUILD_MO(po/opencpn_sv_SE.po sv)
	GETTEXT_BUILD_MO(po/opencpn_et_ET.po et)
	GETTEXT_BUILD_MO(po/opencpn_pt_PT.po pt_PT)
	GETTEXT_BUILD_MO(po/opencpn_pt_BR.po pt_BR)
	GETTEXT_BUILD_MO(po/opencpn_nb_NO.po nb_NO)
	GETTEXT_BUILD_MO(po/opencpn_tr_TR.po tr_TR)
	GETTEXT_BUILD_MO(po/opencpn_fi_FI.po fi_FI)
	GETTEXT_BUILD_MO(po/opencpn_el_GR.po el_GR)
	GETTEXT_BUILD_MO(po/opencpn_zh_TW.po zh_TW)
	GETTEXT_BUILD_MO(po/opencpn_hu_HU.po hu_HU)
	GETTEXT_BUILD_MO(po/opencpn_gl_ES.po gl_ES)
	GETTEXT_BUILD_MO(po/opencpn_ca_ES.po ca_ES)
	ADD_CUSTOM_TARGET(i18n COMMENT "i18n: Done." DEPENDS ${_gmoFiles})
	#ADD_DEPENDENCIES(i18n ${PACKAGE_PO_FILES})
	ADD_DEPENDENCIES(${PACKAGE_NAME} i18n)
ENDIF(GETTEXT_MSGFMT_EXECUTABLE)


# On Windows builds, copy the wxWidgets dlls and helpers to the executable directory
IF(WIN32 AND NOT UNIX)
	INSTALL(FILES "${wxWidgets_LIB_DIR}/wxbase28u_net_vc_custom.dll" DESTINATION ".")
	INSTALL(FILES "${wxWidgets_LIB_DIR}/wxbase28u_vc_custom.dll"     DESTINATION ".")
	INSTALL(FILES "${wxWidgets_LIB_DIR}/wxbase28u_xml_vc_custom.dll" DESTINATION ".")
	INSTALL(FILES "${wxWidgets_LIB_DIR}/wxmsw28u_adv_vc_custom.dll"  DESTINATION ".")
	INSTALL(FILES "${wxWidgets_LIB_DIR}/wxmsw28u_aui_vc_custom.dll"  DESTINATION ".")
	INSTALL(FILES "${wxWidgets_LIB_DIR}/wxmsw28u_core_vc_custom.dll" DESTINATION ".")
	INSTALL(FILES "${wxWidgets_LIB_DIR}/wxmsw28u_gl_vc_custom.dll"   DESTINATION ".")
	INSTALL(FILES "${wxWidgets_LIB_DIR}/wxmsw28u_html_vc_custom.dll" DESTINATION ".")

	if (MSVC10)
		INSTALL(DIRECTORY buildwin/vc10/ DESTINATION ".\\\\vc10")
	elseif(MSVC11)
		INSTALL(DIRECTORY buildwin/vc11/ DESTINATION ".\\\\vc11")
	endif()

	INSTALL(FILES "${CMAKE_SOURCE_DIR}/buildwin/crashrpt/CrashRpt1401.dll"    DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/buildwin/crashrpt/CrashSender1401.exe" DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/buildwin/crashrpt/crashrpt_lang.ini"   DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/buildwin/crashrpt/dbghelp.dll"         DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/buildwin/crashrpt/PrivacyPolicy.txt"   DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/buildwin/ocpn_gltest1.exe"             DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
ENDIF(WIN32 AND NOT UNIX)

MESSAGE (STATUS "")

IF(NOT SKIP_PLUGINS)
	add_subdirectory(plugins)
ENDIF(NOT SKIP_PLUGINS)

add_custom_target(unittest)
macro(run_unittest unittest_target)
	add_custom_target(${unittest_target}_runtest
		COMMAND ${unittest_target}
		DEPENDS ${unittest_target}
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		)
	add_dependencies(unittest ${unittest_target}_runtest)
endmacro()

add_subdirectory(src_test)

# build a CPack driven installer package
SET(CPACK_PACKAGE_NAME "OpenCPN")
SET(CPACK_PACKAGE_VENDOR "opencpn.org")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenCPN ${PACKAGE_VERSION}")
SET(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_CURRENT_BINARY_DIR};${PACKAGE_NAME};ALL;/")
SET(CPACK_PACKAGE_EXECUTABLES ${PACKAGE_NAME} "OpenCPN")


IF(WIN32 AND NOT UNIX)
	# There is a bug in NSI that does not handle full unix paths properly. Make
	# sure there is at least one set of four (4) backlasshes.

	SET(CPACK_NSIS_INSTALLED_ICON_NAME "opencpn.exe")
	SET(CPACK_NSIS_PACKAGE_NAME_LC "opencpn")

	# These lines set the name of the Windows Start Menu shortcut and the icon that goes with it
	SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${PACKAGE_VERSION}")
	SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/src/bitmaps\\\\opencpn.ico")
	SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}_setup" )
	SET(CPACK_NSIS_DIR "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode")
	SET(CPACK_BUILDWIN_DIR "${CMAKE_SOURCE_DIR}/buildwin")
	set(CPACK_BUILDWIN_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/buildwin")

	# Install the VC Redistributable
	SET(vc_redist "vcredist_x86.exe")

	if(MSVC11)
		set(VC_DIR "vc11")
	else()
		set(VC_DIR "vc10")
	endif()

	STRING(REPLACE "/" "\\\\" Temp_vc_redist ${vc_redist})
	GET_FILENAME_COMPONENT(VC_FILE_NAME ${vc_redist} NAME)
	SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS ${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
			"ExecWait '\\\"$INSTDIR\\\\${VC_DIR}\\\\${VC_FILE_NAME}\\\" /q /norestart' $0\\n"
			" Remove folder\\n  RMDir /r \\\"$INSTDIR\\\\${VC_DIR}\\\"\\n\\n" )

ELSE(WIN32 AND NOT UNIX)
	SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${PACKAGE_NAME})
	SET(CPACK_STRIP_FILES "bin/opencpn.exe")
	SET(CPACK_SOURCE_STRIP_FILES "")
	SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_RELEASE}_${ARCH}")
ENDIF(WIN32 AND NOT UNIX)

SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/data/license.txt")
IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README")
	SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
	SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
ENDIF()

SET(CPACK_SOURCE_GENERATOR "TGZ")

# The following components are regex's to match anywhere (unless anchored)
# in absolute path + filename to find files or directories to be excluded
# from source tarball.
set(CPACK_SOURCE_IGNORE_FILES
	"\\\\.cvsignore$"
	"^${CMAKE_CURRENT_SOURCE_DIR}.*/CVS/"
	"^${CMAKE_CURRENT_SOURCE_DIR}/build*"
	"^${CPACK_PACKAGE_INSTALL_DIRECTORY}/*"
	)


IF(UNIX AND NOT APPLE)
	SET(CPACK_GENERATOR ${PACKAGE_FORMAT})
	SET(CPACK_PACKAGE_CONTACT "David S. Register ")
	SET(CPACK_DEBIAN_PACKAGE_DEPENDS ${PACKAGE_DEPS})
	SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS ${PACKAGE_RECS})
	SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
	SET(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
	SET(CPACK_DEBIAN_PACKAGE_SECTION "misc")
	SET(CPACK_RPM_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
	SET(CPACK_RPM_PACKAGE_ARCHITECTURE  ${ARCH})
	SET(CPACK_RPM_PACKAGE_REQUIRES  ${PACKAGE_DEPS})
	SET(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_SOURCE_DIR}/opencpn.spec.in")
	SET(CPACK_OPENCPN_RPM_BINDIR "${CMAKE_INSTALL_PREFIX}/${PREFIX_BIN}")
	SET(CPACK_OPENCPN_RPM_LIBDIR "${PREFIX_LIB}")
	SET(CPACK_OPENCPN_RPM_DATADIR "${CMAKE_INSTALL_PREFIX}/${PREFIX_DATA}")
	SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenSource Chart Plotter/Navigator")
	SET(CPACK_PACKAGE_DESCRIPTION "OpenCPN is a concise ChartPlotter/Navigator. The application supports: GPS/GPDS Position Input, BSB Raster Chart Display, S57 Vector ENChart Display, AIS Input Decoding, Waypoint Autopilot Navigation .")
	SET(CPACK_SET_DESTDIR ON)
	SET(CPACK_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
ENDIF(UNIX AND NOT APPLE)

IF(WIN32 AND NOT UNIX)
	configure_file("${CMAKE_SOURCE_DIR}/src/opencpn.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/src/opencpn.rc")
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_GERMAN.nsh.in"       "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_GERMAN.nsh"       @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_FRENCH.nsh.in"       "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_FRENCH.nsh"       @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_CZECH.nsh.in"        "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_CZECH.nsh"        @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_DANISH.nsh.in"       "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_DANISH.nsh"       @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_SPANISH.nsh.in"      "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_SPANISH.nsh"      @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_ITALIAN.nsh.in"      "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_ITALIAN.nsh"      @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_DUTCH.nsh.in"        "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_DUTCH.nsh"        @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_POLISH.nsh.in"       "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_POLISH.nsh"       @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_PORTUGUESEBR.nsh.in" "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_PORTUGUESEBR.nsh" @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_PORTUGUESE.nsh.in"   "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_PORTUGUESE.nsh"   @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_RUSSIAN.nsh.in"      "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_RUSSIAN.nsh"      @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_SWEDISH.nsh.in"      "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_SWEDISH.nsh"      @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_FINNISH.nsh.in"      "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_FINNISH.nsh"      @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_NORWEGIAN.nsh.in"    "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_NORWEGIAN.nsh"    @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_CHINESETW.nsh.in"    "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_CHINESETW.nsh"    @ONLY)
	configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/LanguageFiles/Langstrings_TURKISH.nsh.in"      "${CPACK_BUILDWIN_BINARY_DIR}/NSIS_Unicode/Include/Langstrings_TURKISH.nsh"      @ONLY)
ENDIF(WIN32 AND NOT UNIX)

if(NOT SKIP_VERSION_CONFIG)
	configure_file(version.h.in "${CMAKE_CURRENT_BINARY_DIR}/src/version.h")
endif(NOT SKIP_VERSION_CONFIG)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/src")

# this dummy target is necessary to make sure the ADDITIONAL_MAKE_CLEAN_FILES directive is executed.
# apparently, the base CMakeLists.txt file must have "some" target to activate all the clean steps.
ADD_CUSTOM_TARGET(dummy COMMENT "dummy: Done." DEPENDS ${PACKAGE_NAME})

SET_DIRECTORY_PROPERTIES(dummy ADDITIONAL_MAKE_CLEAN_FILES version.h)

INCLUDE(CPack)

IF(APPLE)
	# -- Run the BundleUtilities cmake code
	SET(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/buildosx/Info.plist.in")

	SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app")
	SET(DIRS "")

	SET(LIBS "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/PlugIns/libdashboard_pi.dylib")
	SET(LIBS
			${LIBS} "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/PlugIns/libgrib_pi.dylib")

	INSTALL(CODE "
			include(BundleUtilities)
			fixup_bundle(\"${APPS}\"   \"${LIBS}\"   \"${DIRS}\")
			" COMPONENT Runtime)

	ADD_CUSTOM_COMMAND(
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dmg
			COMMAND ${CMAKE_SOURCE_DIR}/buildosx/create-dmg ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dmg ${CMAKE_INSTALL_PREFIX}/bin/
			DEPENDS ${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			COMMENT "create-dmg [${PACKAGE_NAME}]: Generated dmg file."
			)

	ADD_CUSTOM_TARGET(create-dmg COMMENT "create-dmg: Done." DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dmg)

ENDIF(APPLE)

MESSAGE(STATUS "Writing spec file...")

