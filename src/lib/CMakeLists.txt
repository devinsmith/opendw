cmake_minimum_required(VERSION 3.7.2)
project(opendw)
set(TARGET_NAME dragon)

# C and C++ sources are freely mixed.
set(SOURCES
  bithelp.c bufio.c compress.c engine.c log.c offsets.c player.c
  resource.c state.c tables.c ui.c utils.c vga.c)

add_library(${TARGET_NAME} STATIC ${SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)

#if(MSVC)
#  target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
#else()
#  target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror)
target_compile_options(${TARGET_NAME} PRIVATE -Wall)
#endif()
