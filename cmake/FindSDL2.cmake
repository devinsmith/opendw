find_path(SDL2_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL2 ${CMAKE_SOURCE_DIR}/deps/sdl2/include)
set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

FIND_LIBRARY(SDL2_LIBRARY NAMES SDL2 PATH_SUFFIXES lib ${CMAKE_SOURCE_DIR}/deps/sdl2/${VC_LIB_PATH_SUFFIX})
IF(SDL2_LIBRARY)
  SET(SDL2_LIBRARIES ${SDL2_LIBRARIES} ${SDL2_LIBRARY})
ENDIF(SDL2_LIBRARY)

IF(WIN32)
  # MinGW needs an additional library, mwindows
  # It's total link flags should look like -lmingw32 -lSDL2TTFmain -lSDL2TTF -lmwindows
  # (Actually on second look, I think it only needs one of the m* libraries.)
  IF(MINGW)
    SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
  ENDIF(MINGW)
  FIND_LIBRARY(SDL2_MAIN_LIBRARY NAMES SDL2main PATH_SUFFIXES lib ${CMAKE_SOURCE_DIR}/deps/sdl2/${VC_LIB_PATH_SUFFIX})
  SET(SDL2_LIBRARIES ${MINGW32_LIBRARY} ${SDL2_MAIN_LIBRARY} ${SDL2_LIBRARIES})
ENDIF(WIN32)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2 REQUIRED_VARS SDL2_LIBRARIES SDL2_INCLUDE_DIRS)
MARK_AS_ADVANCED(SDL2_LIBRARY SDL2_MAIN_LIBRARY SDL2_INCLUDE_DIR)
