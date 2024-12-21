@echo off

cmd /c "emcc -c ../src/basisu.cpp -I../include -pthread -o basisu.o"
cmd /c "emcc -c ../include/basisu/basisu_transcoder.cpp -I../include -pthread -o basisu_transcoder.o"
mkdir ..\lib\web
emar -rcs ../lib/web/libbasisu.a basisu.o basisu_transcoder.o
