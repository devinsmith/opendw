# OpenDW
A game engine for Interplay's 1989/1990 Dragon Wars game.
Original data files are required.
This game was originally written in 16 bit x86 assembly language running in
real mode. OpenDW is an attempt to port this game to modern environments using
the C language.

Original game engine by [Rebecca Ann Heineman](https://www.burgerbecky.com/).

This game can be purchased at [GOG](https://www.gog.com/game/dragon_wars).

# Prerequisites
- Executable (DRAGON.COM).
- Original data files (DATA1 and DATA2).
- SDL2.

Under Linux/Unix operating systems, this engine expects that the original game
files should be all lowercase (e.g. DRAGON.COM -> dragon.com). The files
dragon.com, data1, and data2 should be in the same directory with the build.

# Building

Install dependencies first:

VoidLinux

```
sudo xbps-install -S SDL2-devel
```

Debian

```
sudo apt install libsdl2-dev
```

Then use CMake to build:

```
mkdir build
cd build
cmake ..
make
```

The `sdldragon` binary will be in build/src/fe/sdldragon


# Screenshot

![Title screen](img/dw.png)

