# Makefile for dragon wars (core lib)

.PHONY: all clean

SRCS = bufio.c compress.c engine.c log.c offsets.c \
			 player.c resource.c state.c tables.c ui.c utils.c

DEP_INCLUDES = -I.
DEP_LIBS =

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

AR? = ar
RM? = rm -f
RANLIB = ranlib

# Debugging flags
CFLAGS = -Wall -g3
#CFLAGS = -Wall -O2

LIB = libdragon
STATIC_LIB = $(LIB).a

all: $(STATIC_LIB)

$(STATIC_LIB): $(OBJS)
	$(RM) $(STATIC_LIB)
	$(AR) cru $(STATIC_LIB) $(OBJS)
	$(RANLIB) $(STATIC_LIB)

.c.o:
	$(CC) $(CFLAGS) $(DEP_INCLUDES) -MMD -MP -MT $@ -o $@ -c $<

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(STATIC_LIB)

# Include automatically generated dependency files
-include $(DEPS)

