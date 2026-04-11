# The minimum version is the version supported by the oldest Debian release
# Ubuntu 20.04LTS uses 3.16.3
cmake_minimum_required(VERSION 3.13.4)

project(opendw)
set(TARGET_NAME sdldragon)

# C and C++ sources are freely mixed.
set(SOURCES main.c vga_sdl.c)

add_executable(${TARGET_NAME} ${SOURCES})
list(APPEND INC_DIRS ${SDL2_INCLUDE_DIRS})
list(APPEND LINK_LIBRARIES ${SDL2_LIBRARIES})

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib ${INC_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE dragon ${LINK_LIBRARIES})

# Also build ndragon (OpenDW without any video driver)
add_executable(ndragon main.c vga_null.c)
target_include_directories(ndragon PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)
target_link_libraries(ndragon PRIVATE dragon)

install(TARGETS sdldragon DESTINATION bin)

if(MSVC)
    # Not yet with /WX
    #target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
    target_compile_options(${TARGET_NAME} PRIVATE /W4)
else()
    # Not yet with -Werror
    #target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror)
    target_compile_options(${TARGET_NAME} PRIVATE -Wall)
endif()
