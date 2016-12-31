cmake_minimum_required (VERSION 3.0)
project (cloud-storage)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(CMAKE_BUILD_TYPE debug)


include_directories(${CMAKE_SOURCE_DIR}/modules/rapidjson/include)
include_directories(${CMAKE_SOURCE_DIR}/modules)
include_directories(${CMAKE_SOURCE_DIR}/curl-7.50.1/include)

include_directories (include) 
include_directories (external) 
include_directories (src) 
include_directories (src/client/include)

add_subdirectory (src/cloud)
