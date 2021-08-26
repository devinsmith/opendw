FIND_PATH(SDL2_INCLUDE_DIR SDL2/SDL.h)
IF(SDL2_INCLUDE_DIR)
  SET(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR}/SDL2)
ENDIF(SDL2_INCLUDE_DIR)

FIND_LIBRARY(SDL2_LIBRARY NAMES SDL2)
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
  FIND_LIBRARY(SDL2_MAIN_LIBRARY NAMES SDL2main)
  SET(SDL2_LIBRARIES ${MINGW32_LIBRARY} ${SDL2_MAIN_LIBRARY} ${SDL2_LIBRARIES})
ENDIF(WIN32)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2 REQUIRED_VARS SDL2_LIBRARIES SDL2_INCLUDE_DIRS)
MARK_AS_ADVANCED(SDL2_LIBRARY SDL2_MAIN_LIBRARY SDL2_INCLUDE_DIR)
