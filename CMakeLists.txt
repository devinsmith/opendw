# The minimum version is the version supported by the oldest Debian release
# Ubuntu 20.04LTS uses 3.16.3
cmake_minimum_required(VERSION 3.13.4)
project(opendw)

set(PACKAGE opendw)
set(VERSION 0.02)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

# Currently the only supported output is SDL2.
find_package(SDL2 REQUIRED)
message (SDL2_FOUND=${SDL2_FOUND})
message (SDL2_INC_DIRS=${SDL2_INCLUDE_DIRS})
message (SDL2_INC_DIR=${SDL2_INCLUDE_DIR})
message (SDL2_LIBS=${SDL2_LIBRARIES})
message (SDL2_LIB=${SDL2_LIBRARY})
#find_package(X11)

add_subdirectory(src)
