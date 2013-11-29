project(wxjson)
cmake_minimum_required(VERSION 2.8)

#if(NOT WIN32 AND NOT APPLE)
#	add_definitions("-Wall -Wextra -fexceptions -rdynamic")
#	add_definitions("-g -fno-strict-aliasing")
#
#	set(CMAKE_C_FLAGS "-O2 -Wall -Wextra -fPIC")
#	set(CMAKE_C_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra -fPIC")
#	set(CMAKE_C_FLAGS_MINSIZEREL "-O2 -Wall -Wextra -fPIC")
#	set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -Wextra -fPIC")
#	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-ggdb -O3 -Wall -Wextra -fPIC")
#
#	set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra -fPIC")
#	set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra -fPIC")
#	set(CMAKE_CXX_FLAGS_MINSIZEREL "-O2 -Wall -Wextra -fPIC")
#	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -fPIC")
#	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-ggdb -O3 -Wall -Wextra -fPIC")
#
#	set(CMAKE_EXE_LINKER_FLAGS "-O2 -Wall -Wextra")
#	set(CMAKE_SHARED_LINKER_FLAGS "-O2 -Wall -Wextra")
#	set(CMAKE_MODULE_LINKER_FLAGS "-O2 -Wall -Wextra")
#endif()

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

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

