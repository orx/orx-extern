@echo off

cmd /c "emcmake cmake -DCMAKE_BUILD_TYPE=Release -DEMSCRIPTEN_GLFW3_DISABLE_MULTI_WINDOW_SUPPORT=ON ."
cmd /c "emmake make -j"

mkdir lib
move libglfw3.a lib

copy src\js\lib_emscripten_glfw3.js lib
