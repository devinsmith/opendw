# Makefile for dragon wars

.PHONY: all clean

SRCS = main.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

DEP_INCLUDES = -I. $(shell pkg-config sdl2 --cflags)
DEP_LIBS = $(shell pkg-config sdl2 --libs)

CFLAGS = -Wall -O3
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

