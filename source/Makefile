# Makefile for dragon wars

.PHONY: all clean

SRCS = bufio.c compress.c engine.c main.c offsets.c \
			 resource.c tables.c ui.c utils.c

DEP_INCLUDES = -I.
DEP_LIBS =

# SDL VGA implementation.
SRCS += vga_sdl.c
DEP_INCLUDES += $(shell pkg-config sdl2 --cflags)
DEP_LIBS += $(shell pkg-config sdl2 --libs)

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

# Debugging flags
#CFLAGS = -Wall -g3 -DNO_DISPLAY
CFLAGS = -Wall -O2
EXE = dragon

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(DEP_LFLAGS) $(DEP_LIBS)

.c.o:
	$(CC) $(CFLAGS) $(DEP_INCLUDES) -MMD -MP -MT $@ -o $@ -c $<

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(EXE)

# Include automatically generated dependency files
-include $(DEPS)

