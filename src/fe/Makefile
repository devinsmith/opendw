# Makefile for dragon wars

.PHONY: all clean

SRCS = main.c

# VGA drivers
NULL_SRC = vga_null.c
SDL_SRC = vga_sdl.c
X_SRC = vga_xlib.c

VGA_OBJS = vga_null.o vga_sdl.o vga_xlib.o

DEP_INCLUDES = -I. -I../lib
DEP_LIBS = -L../lib -ldragon

# SDL VGA implementation.
SDL_INCLUDES = $(shell pkg-config sdl2 --cflags)
SDL_LIBS = $(shell pkg-config sdl2 --libs)

X_LIBS = -lX11

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d) $(VGA_OBJS:.o=.d)

# Debugging flags
CFLAGS = -Wall -g3
#CFLAGS = -Wall -O2

EXES = sdldragon ndragon

# If you have X, uncomment this line.
EXES += xdragon

all: $(EXES)

sdldragon: $(OBJS) vga_sdl.o
	$(CC) $(CFLAGS) -o $@ $(OBJS) vga_sdl.o $(DEP_LIBS) $(SDL_LIBS)

xdragon: $(OBJS) vga_xlib.o
	$(CC) $(CFLAGS) -o $@ $(OBJS) vga_xlib.o $(DEP_LIBS) $(X_LIBS)

ndragon: $(OBJS) vga_null.o
	$(CC) $(CFLAGS) -o $@ $(OBJS) vga_null.o $(DEP_LIBS)

vga_sdl.o: vga_sdl.c
	$(CC) $(CFLAGS) $(DEP_INCLUDES) $(SDL_INCLUDES) -MMD -MP -MT $@ -o $@ -c vga_sdl.c

vga_xlib.o: vga_xlib.c
	$(CC) $(CFLAGS) $(DEP_INCLUDES) -MMD -MP -MT $@ -o $@ -c vga_xlib.c

vga_null.o: vga_null.c
	$(CC) $(CFLAGS) $(DEP_INCLUDES) -MMD -MP -MT $@ -o $@ -c vga_null.c

.c.o:
	$(CC) $(CFLAGS) $(DEP_INCLUDES) -MMD -MP -MT $@ -o $@ -c $<

clean:
	rm -f $(VGA_OBJS)
	rm -f $(DEPS)
	rm -f $(EXES)

# Include automatically generated dependency files
-include $(DEPS)

