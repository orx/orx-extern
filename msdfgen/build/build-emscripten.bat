@echo off

cmd /c "emcc -DMSDFGEN_NO_FREETYPE -c ../src/msdfgen/msdfgen.cpp -I../include -o msdfgen.o"
cmd /c "emcc -DMSDFGEN_NO_FREETYPE -c ../src/msdfgen_c.cpp -I../include -o msdfgen_c.o"
mkdir ..\lib\web
emar -rcs ../lib/web/libmsdfgen.a msdfgen.o msdfgen_c.o
