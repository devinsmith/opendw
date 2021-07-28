cmake_minimum_required(VERSION 3.7.2)
project(opendw)

set(PACKAGE opendw)
set(VERSION 0.02)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

# Currently the only supported output is SDL2.
find_package(SDL2 REQUIRED)
#find_package(X11)

add_subdirectory(src)
