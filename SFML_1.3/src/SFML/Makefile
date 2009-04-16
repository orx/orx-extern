export CC        = gcc
export CPP       = g++
export CFLAGS    = -W -Wall -pedantic -I../../../include -I../../ -DNDEBUG -O2 -fPIC
export CFLAGSEXT = -I../../../include -I../.. -DNDEBUG -O2 -fPIC
export LDFLAGS   = -shared
export LIBPATH   = ../../../lib
export VERSION   = 1
export CP        = cp
export LN        = ln
export LNFLAGS   = -s -f
export AR        = ar
export ARFLAGS   = rcs

ifeq ($(static), yes)
    CFLAGS    = -W -Wall -pedantic -I../../../include -I../../ -DNDEBUG -O2
    CFLAGSEXT = -I../../../include -I../.. -DNDEBUG -O2
    LDFLAGS   = 
endif

all: sfml-system sfml-window sfml-graphics sfml-audio

sfml-system:
	@(cd ./System && $(MAKE))

sfml-window:
	@(cd ./Window && $(MAKE))

sfml-network:
	@(cd ./Network && $(MAKE))

sfml-graphics:
	@(cd ./Graphics && $(MAKE))

sfml-audio:
	@(cd ./Audio && $(MAKE))

.PHONY: clean mrproper

clean:
	@(cd ./System && $(MAKE) $@	&& cd ../Window && $(MAKE) $@ && cd ../Network && $(MAKE) $@ && cd ../Graphics && $(MAKE) $@ && cd ../Audio && $(MAKE) $@)

mrproper: clean
	@(cd ./System && $(MAKE) $@	&& cd ../Window && $(MAKE) $@ && cd ../Network && $(MAKE) $@ && cd ../Graphics && $(MAKE) $@ && cd ../Audio && $(MAKE) $@)

install:
	@(cd ./System && $(MAKE) $@	&& cd ../Window && $(MAKE) $@ && cd ../Network && $(MAKE) $@ && cd ../Graphics && $(MAKE) $@ && cd ../Audio && $(MAKE) $@ && $(CP) -r ../../../include/SFML/ /usr/include/)
