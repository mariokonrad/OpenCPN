message(STATUS "Extern: Pugixml 1.2")

set(pugixml_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/local")
set(pugixml_INCLUDE_DIR "${pugixml_INSTALL_DIR}/include")
set(pugixml_LIBRARY_DIR "${pugixml_INSTALL_DIR}/lib")

ExternalProject_Add(extern_pugixml
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/pugixml"
	# configure
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src_extern/pugixml-1.2/scripts"
	CMAKE_ARGS
		-DCMAKE_INSTALL_PREFIX=${pugixml_INSTALL_DIR}
		-DBUILD_SHARED_LIBS=OFF
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
		-DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
		-DCMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}
		-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
		-DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
		-DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
	# install
	INSTALL_DIR ${pugixml_INSTALL_DIR}
	)

add_library(pugixml STATIC IMPORTED)
set_target_properties(pugixml
	PROPERTIES
		IMPORTED_LOCATION
			${pugixml_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}pugixml${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
add_dependencies(pugixml extern_pugixml)
include_directories(${pugixml_INCLUDE_DIR})

