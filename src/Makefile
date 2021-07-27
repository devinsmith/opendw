# Master makefile for opendw

.PHONY: all clean

all:
	@(cd lib && $(MAKE))
	@(cd fe && $(MAKE))

clean:
	@(cd lib && $(MAKE) clean)
	@(cd fe && $(MAKE) clean)

