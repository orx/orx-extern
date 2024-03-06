# Building GLFW3

## Windows

### Visual Studio

#### VS2017

1. Run `cmake -G "Visual Studio 15 2017" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF -DGLFW_USE_HYBRID_HPG=ON .` in the root directory
2. Open `GLFW.sln`
3. Edit `glfw`'s project properties
    - Floating point model -> fast (`/fpfast`)
    - In Librarian->All Options, remove `/machine:X86 %(AdditionalOptions)`
    - In Librarian->General, select `Ignore All Default Libraries` -> `Yes`
    - Add `x64` as a target in the configuration manager, based on `Win32`
4. Build both Release configurations & copy both `glfw3.lib`

### MinGW

1. Run `cmake -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_USE_HYBRID_HPG=ON .` in the root directory
2. Edit `CMakeCache.txt`
    - Set `CMAKE_BUILD_TYPE:STRING=Release`
    - Set `CMAKE_C_FLAGS_RELEASE:STRING=-O3 -DNDEBUG -m64`
3. Run `mingw32-make` & copy `src/libglfw3.a` (64 bit)
4. Edit `CMakeCache.txt`
    - Set `CMAKE_C_FLAGS_RELEASE:STRING=-O3 -DNDEBUG -m32`
5. Run `mingw32-make` & copy `src/libglfw3.a` (32 bit)
For both 32bit & 64bit versions:
6. On a `Linux` machine (or using `binutils` for `Windows`), copy `libglfw3.a` + `libopengl32.a` & `libgdi32.a` (from MinGW installation) to the same directory
7. Run `ar xv libopengl32.a | cut -f3 -d ' ' | xargs ar rvs libglfw3.a && rm *.o && echo 'done'`
8. Run `ar xv libgdi32.a | cut -f3 -d ' ' | xargs ar rvs libglfw3.a && rm *.o && echo 'done'`
9. Copy `libglfw3.a` back to the `Windows` machine
10. Run `ranlib libglfw3.a`

## Linux

1. Run `cmake -G "Unix Makefiles" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF .` in the root directory
2. Edit `CMakeCache.txt`
    - Set `CMAKE_BUILD_TYPE:STRING=Release`
    - Set `CMAKE_C_FLAGS_RELEASE:STRING=-O3 -DNDEBUG -m64`
3. Run `make` & copy `src/libglfw3.a` (64 bit)
4. Edit `CMakeCache.txt`
    - Set `CMAKE_C_FLAGS_RELEASE:STRING=-O3 -DNDEBUG -m32`
5. Run `make` & copy `src/libglfw3.a` (32 bit)

## OSX

1. Add `set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)` to the `Cocoa` section of `CMakeLists.txt`
2. Add `set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)` to the `Cocoa` section of `CMakeLists.txt`
3. Run `cmake -G "Unix Makefiles" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF .` in the root directory
4. Edit `CMakeCache.txt`
    - Set `CMAKE_BUILD_TYPE:STRING=Release`
5. Run `make` & copy `src/libglfw3.a` (Universal 64 bit)
