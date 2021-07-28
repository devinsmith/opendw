cmake_minimum_required(VERSION 3.7.2)

project(opendw)
find_package(Check REQUIRED)
# Check requires threads.
find_package(Threads REQUIRED)

enable_testing()

set(SOURCES test_bithelp.cpp)

add_executable(test_bithelp ${SOURCES})
target_include_directories(test_bithelp PRIVATE ${CMAKE_SOURCE_DIR}/src/lib ${CHECK_INCLUDE_DIRS})
target_link_libraries(test_bithelp PUBLIC dragon ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_test(NAME test_bithelp COMMAND test_bithelp)
#if(MSVC)
#  target_compile_options(test_bithelp PRIVATE /W4 /WX)
#else()
#  target_compile_options(test_bithelp PRIVATE -Wall -Werror)
#endif()

