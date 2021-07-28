cmake_minimum_required(VERSION 3.7.2)

project(opendw)
set(TARGET_NAME sdldragon)

# C and C++ sources are freely mixed.
set(SOURCES main.c vga_sdl.c)

add_executable(${TARGET_NAME} ${SOURCES})

list(APPEND INC_DIRS ${SDL2_INCLUDE_DIRS})
list(APPEND LINK_LIBRARIES ${SDL2_LIBRARIES})

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib ${INC_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE dragon ${LINK_LIBRARIES})

#if(MSVC)
#  target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
#else()
#target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror)
#target_compile_options(${TARGET_NAME} PRIVATE -Wall)
#endif()

