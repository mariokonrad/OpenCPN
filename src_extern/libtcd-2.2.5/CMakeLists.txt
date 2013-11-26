project(libtcd)
cmake_minimum_required(VERSION 2.8)

# TODO: this is platform specific (especially for windows)
set(sub_systypes "#include <sys/types.h>")
set(sub_inttypes "#include <stdint.h>")

set(configure_input "configured by cmake")

set(libtcd_major_rev "2")
set(libtcd_minor_rev "2")
set(libtcd_patchlevel ".5")
set(ver_COMPAT114 "")
set(libtcd_rev_date "2010-08-17")

set(sub_int8_t    "int8_t")
set(sub_int16_t   "int16_t")
set(sub_int32_t   "int32_t")
set(sub_int64_t   "int64_t")
set(sub_uint8_t   "uint8_t")
set(sub_uint16_t  "uint16_t")
set(sub_uint32_t  "uint32_t")
set(sub_uint64_t  "uint64_t")

set(def_COMPAT114 "undef")
set(def_PRAGMAMESSAGE "undef")

configure_file(tcd.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/tcd.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)

add_library(tcd
	STATIC
	tide_db.c
	bit_pack.c
	)

set_target_properties(tcd
	PROPERTIES VERSION 2.2.5-r3
	SOVERSION 2.2.5
	)

install(
	TARGETS tcd
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/src/tcd.h
	DESTINATION include/libtcd
	)

