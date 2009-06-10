SRC = $(wildcard *.cpp ./Unix/*.cpp)
OBJ = $(SRC:.cpp=.o)

ifeq ($(STATIC), yes)
    LIB     = libsfml-system.a
    LIBNAME = $(LIBPATH)/$(LIB)
    INSTALL = 
else
    LIB     = libsfml-system.so
    LIBNAME = $(LIBPATH)/$(LIB).$(VERSION)
    INSTALL = && $(LN) $(LNFLAGS) $(DESTLIBDIR)/$(LIB).$(VERSION) $(DESTLIBDIR)/$(LIB)
endif

all: $(LIB)

libsfml-system.a: $(OBJ)
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJ)

libsfml-system.so: $(OBJ)
	$(CPP) $(LDFLAGS) -Wl,-soname,$(LIB).$(VERSION) -o $(LIBNAME) $(OBJ) -lpthread

$(OBJ): %.o: %.cpp
	$(CPP) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	@rm -rf $(OBJ)

mrproper: clean
	@rm -rf $(LIBNAME)

install:
	@($(CP) $(LIBNAME) $(DESTLIBDIR) $(INSTALL))
