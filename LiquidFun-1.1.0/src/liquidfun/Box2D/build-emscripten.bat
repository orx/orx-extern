@echo off

cmd /c "emcmake cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ."
cmd /c "emmake make -j"

mkdir ..\..\..\lib\web
move Box2D\libliquidfun.a ..\..\..\lib\web
