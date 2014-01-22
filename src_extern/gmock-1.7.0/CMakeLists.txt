project(gmock)
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(gmock
	STATIC
	custom_main.cpp
	gmock-gtest-all.cc
	)

set_target_properties(gmock
	PROPERTIES VERSION 1.7.0
	SOVERSION 1.7.0
	)

install(
	TARGETS gmock
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

install(
	FILES gmock/gmock.h
	DESTINATION include/gmock
	)

install(
	FILES gtest/gtest.h
	DESTINATION include/gtest
	)

