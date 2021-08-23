cmake_minimum_required(VERSION 3.7.2)

project(opendw)
find_package(Check REQUIRED)
# Check requires threads.
find_package(Threads REQUIRED)

set(TARGET_NAME opendw_test)

set(SOURCES test_compress.cpp test_opendw.cpp test_vga.cpp)

add_executable(${TARGET_NAME} ${SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib ${CHECK_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC dragon ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})

if(MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror)
endif()

