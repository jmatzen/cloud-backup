cmake_minimum_required (VERSION 3.0)
project (cloud)

add_library(cryptopp STATIC IMPORTED)
set_property(TARGET cryptopp PROPERTY IMPORTED_LOCATION_DEBUG 
  ${CMAKE_SOURCE_DIR}/modules/cryptopp/win32/output/debug/cryptlib.lib)

add_library(curl STATIC IMPORTED)
set_property(TARGET curl PROPERTY IMPORTED_LOCATION_DEBUG 
  ${CMAKE_SOURCE_DIR}/curl-7.50.1/builds/libcurl-vc14-x86-debug-dll-ipv6-sspi-winssl/lib/libcurl_debug.lib)

file (GLOB files *.cpp *.h)

add_executable(cloud ${files})

target_link_libraries(cloud debug cryptopp curl ws2_32)
