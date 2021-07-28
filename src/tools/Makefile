# Makefile for tools

.PHONY: all clean

C_SRCS =
CPP_SRCS = disasm.cpp resextract.cpp

OBJS = $(C_SRCS:.c=.o) $(CPP_SRCS:.cpp=.o)
DEPS = $(C_SRCS:.c=.d) $(CPP_SRCS:.cpp=.d)

DEP_INCLUDES = -I../lib
DEP_LIBS = -L../lib -ldragon

CFLAGS = -Wall -g3
EXES = disasm resextract

all: $(EXES)

disasm: disasm.o
	$(CXX) $(CFLAGS) -o $@ $< $(DEP_LFLAGS) $(DEP_LIBS)

# XXX: Remove requirement to link a vga driver in for extractor
resextract: resextract.o
	$(CXX) $(CFLAGS) -o $@ resextract.o ../fe/vga_null.o $(DEP_LFLAGS) $(DEP_LIBS)

.c.o:
	$(CC) $(CFLAGS) $(DEP_INCLUDES) -MMD -MP -MT $@ -o $@ -c $<

.cpp.o:
	$(CXX) $(CFLAGS) $(DEP_INCLUDES) -std=c++11 -MMD -MP -MT $@ -o $@ -c $<

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(EXE)

# Include automatically generated dependency files
-include $(DEPS)

