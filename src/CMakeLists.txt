cmake_minimum_required(VERSION 3.7.2)
project(opendw)

option(ENABLE_TOOLS "Enable developer tools" OFF)
option(ENABLE_TESTS "Enable tests" OFF)

add_subdirectory(lib)
add_subdirectory(fe)
if (ENABLE_TOOLS)
  add_subdirectory(tools)
endif()
if (ENABLE_TESTS)
  add_subdirectory(tests)
endif()