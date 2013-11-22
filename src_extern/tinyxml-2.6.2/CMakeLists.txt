project(tinyxml)
cmake_minimum_required(VERSION 2.8)

option(USE_STL "Use STL" ON)

set(SOURCES
	tinyxml.h
	tinystr.h
	tinyxml.cpp
	tinyxmlparser.cpp
	tinyxmlerror.cpp
	tinystr.cpp
	)

if(USE_STL)
	message(STATUS "### tinyxml with stl")
	add_definitions("-DTIXML_USE_STL")
else()
	message(STATUS "### tinyxml without stl")
endif()

add_library(tinyxml STATIC ${SOURCES})

set_target_properties(tinyxml PROPERTIES VERSION 2.6.2 SOVERSION 2.6.2)

install(TARGETS tinyxml LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES tinyxml.h DESTINATION include/tinyxml)

if(NOT USE_STL)
	install(FILES tinystr.h DESTINATION include/tinyxml)
endif()

