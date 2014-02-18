
message("@@ ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_BUILD_TYPE       : ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_COMPILER       : ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_C_FLAGS          : ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_DEBUG    : ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "CMAKE_C_FLAGS_RELEASE  : ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_COMPILER     : ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS        : ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG  : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_INSTALL_PREFIX   : ${CMAKE_CMAKE_INSTALL_PREFIX}")

file(GLOB_RECURSE TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
message(STATUS "Tests found: ${TEST_NAME}")
foreach(TEST_FILE ${TEST_SRC})
	string(REGEX REPLACE ".*src_test/" "" TEST_NAME ${TEST_FILE})
	message(STATUS ":  ${TEST_NAME}")
endforeach(TEST_FILE)

# TODO: instead of compile/link to individual files, the tests should
#       link against libraries (for example: libgeo instead of geo/Position)
#   OR: the built files should be built with coverage

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
	set(PLATFORM_SRC
		${CMAKE_SOURCE_DIR}/src/windows/compatibility.cpp
		${CMAKE_SOURCE_DIR}/src/windows/compatibility.h
		)
endif()

add_executable(testrunner
	${TEST_SRC}
	${PLATFORM_SRC}
	${CMAKE_SOURCE_DIR}/src/UserColors.cpp
	${CMAKE_SOURCE_DIR}/src/navigation/AnchorDist.cpp
	${CMAKE_SOURCE_DIR}/src/navigation/MagneticVariation.cpp
	${CMAKE_SOURCE_DIR}/src/geo/Position.cpp
	${CMAKE_SOURCE_DIR}/src/geo/BoundingBox.cpp
	${CMAKE_SOURCE_DIR}/src/geo/Polygon.cpp
	${CMAKE_SOURCE_DIR}/src/geo/LatLonBoundingBox.cpp
	${CMAKE_SOURCE_DIR}/src/util/Vector2D.cpp
	${CMAKE_SOURCE_DIR}/src/util/ValueFilter.cpp
	${CMAKE_SOURCE_DIR}/src/util/crc32.cpp
	${CMAKE_SOURCE_DIR}/src/util/uuid.cpp
	)

target_link_libraries(testrunner
	global
	${wxWidgets_LIBRARIES}
	gmock
	)

run_unittest(testrunner)

