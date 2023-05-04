cmake_minimum_required(VERSION 3.7.2)

project(opendw)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DISASM_SOURCES disasm.cpp)
set(RESEXT_SOURCES resextract.cpp)
set(STREXT_SOURCES strextract.cpp)
set(PPMWRITE_SOURCES ppm.cpp)
set(MOUSE_SOURCES mouse.cpp)
set(MONSTER_INFO_SOURCES monster_info.cpp)

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

add_executable(strextract ${STREXT_SOURCES})
target_include_directories(strextract PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)
target_link_libraries(strextract PUBLIC dragon)

# Rather useless tool
#add_executable(kbhit kbhit.cpp)

add_executable(ppmwrite ${PPMWRITE_SOURCES})
target_include_directories(ppmwrite PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)
target_link_libraries(ppmwrite PUBLIC dragon)
if(MSVC)
  target_compile_options(ppmwrite PRIVATE /W4 /WX)
else()
  target_compile_options(ppmwrite PRIVATE -Wall -Werror)
endif()

add_executable(mouse ${MOUSE_SOURCES})
target_include_directories(mouse PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)
target_link_libraries(mouse PUBLIC dragon)
if(MSVC)
  target_compile_options(mouse PRIVATE /W4 /WX)
else()
  target_compile_options(mouse PRIVATE -Wall -Werror)
endif()

add_executable(monster_info ${MONSTER_INFO_SOURCES})
target_include_directories(monster_info PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)
target_link_libraries(monster_info PUBLIC dragon)



install(TARGETS disasm DESTINATION bin)
#if(MSVC)
#  target_compile_options(resextract PRIVATE /W4 /WX)
#else()
#  target_compile_options(resextract PRIVATE -Wall -Werror)
#endif()

