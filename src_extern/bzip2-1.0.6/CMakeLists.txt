project(libbzip2)
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

add_definitions(-D_FILE_OFFSET_BITS=64)

add_library(bzip2
	STATIC
	blocksort.c
	huffman.c
	crctable.c
	randtable.c
	compress.c
	decompress.c
	bzlib.c
	)

set_target_properties(bzip2
	PROPERTIES VERSION 1.0.6
	SOVERSION 1.0.6
	)

install(
	TARGETS bzip2
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

install(
	FILES bzlib.h
	DESTINATION include
	)

