project(wxjson)
cmake_minimum_required(VERSION 2.8)

message(STATUS "CMAKE_BUILD_TYPE       : ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_COMPILER       : ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_C_FLAGS          : ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_DEBUG    : ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "CMAKE_C_FLAGS_RELEASE  : ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_COMPILER     : ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS        : ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG  : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions("-DwxJSON_USE_STL")

add_library(wxjson
	STATIC
	src/jsonreader.cpp
	src/jsonval.cpp
	src/jsonwriter.cpp
	)

set_target_properties(wxjson
	PROPERTIES VERSION 1.2.1
	SOVERSION 1.2.1
	)

install(
	TARGETS wxjson
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

install(
	FILES
		include/wx/json_defs.h
		include/wx/jsonreader.h
		include/wx/jsonval.h
		include/wx/jsonwriter.h
	DESTINATION include/wx
	)

