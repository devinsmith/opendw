# The minimum version is the version supported by the oldest Debian release
# Ubuntu 20.04LTS uses 3.16.3
cmake_minimum_required(VERSION 3.13.4)
project(opendw)

option(ENABLE_TOOLS "Enable developer tools" OFF)
option(ENABLE_TESTS "Enable tests" OFF)

# This code uses "insecure" (as determined by Microsoft) CRT functions
# Disable warnings about this across the board.
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(lib)
add_subdirectory(fe)
if (ENABLE_TOOLS)
  add_subdirectory(tools)
endif()
if (ENABLE_TESTS)
  add_subdirectory(tests)
endif()
