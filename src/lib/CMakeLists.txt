cmake_minimum_required(VERSION 3.7.2)
project(opendw)
set(TARGET_NAME dragon)

# C and C++ sources are freely mixed.
set(SOURCES
  bufio.c bufio.h compress.c compress.h engine.c engine.h log.c
  log.h mouse.c mouse.h offsets.c offsets.h player.c player.h
  resource.c resource.h state.c state.h tables.c tables.h
  timers.c timers.h ui.c ui.h
  utils.c utils.h vga.c vga.h)

add_library(${TARGET_NAME} STATIC ${SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)

if(MSVC)
  # Not yet with /WX
  #target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
  target_compile_options(${TARGET_NAME} PRIVATE /W4)
else()
  # Not yet with -Werror
  #target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror)
  target_compile_options(${TARGET_NAME} PRIVATE -Wall)
endif()
