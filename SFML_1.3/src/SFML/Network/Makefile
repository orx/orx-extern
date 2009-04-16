SRC = $(wildcard *.cpp ./Unix/*.cpp)
OBJ = $(SRC:.cpp=.o)

ifeq ($(static), yes)
    LIB     = libsfml-network-s.a
    LIBNAME = $(LIBPATH)/$(LIB)
    INSTALL = 
else
    LIB     = libsfml-network.so
    LIBNAME = $(LIBPATH)/$(LIB).$(VERSION)
    INSTALL = && $(LN) $(LNFLAGS) /usr/lib/$(LIB).$(VERSION) /usr/lib/$(LIB)
endif

all: $(LIB)

libsfml-network-s.a: $(OBJ)
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJ)

libsfml-network.so: $(OBJ)
	$(CPP) $(LDFLAGS) -Wl,-soname,$(LIB).$(VERSION) -o $(LIBNAME) $(OBJ)

$(OBJ): %.o: %.cpp
	$(CPP) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	@rm -rf $(OBJ)

mrproper: clean
	@rm -rf $(LIBNAME)

install:
	@($(CP) $(LIBNAME) /usr/lib $(INSTALL))
