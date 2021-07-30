cmake_minimum_required(VERSION 3.7.2)

project(opendw)

set(DISASM_SOURCES disasm.cpp)
set(RESEXT_SOURCES resextract.cpp)

add_executable(disasm ${DISASM_SOURCES})
target_include_directories(disasm PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)
target_link_libraries(disasm PUBLIC dragon)
if(MSVC)
  target_compile_options(disasm PRIVATE /W4 /WX)
else()
  target_compile_options(disasm PRIVATE -Wall -Werror)
endif()

add_executable(resextract ${RESEXT_SOURCES})
target_include_directories(resextract PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)
target_link_libraries(resextract PUBLIC dragon)
#if(MSVC)
#  target_compile_options(resextract PRIVATE /W4 /WX)
#else()
#  target_compile_options(resextract PRIVATE -Wall -Werror)
#endif()
