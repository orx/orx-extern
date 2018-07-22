# Building GLFW3

## Windows

### Visual Studio

#### VS2015

1. Run `cmake -G "Visual Studio 14 2015" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF -DGLFW_USE_HYBRID_HPG=ON .` in the root
2. Open `GLFW.sln`
3. Edit `glfw`'s project properties
    - Floating point model -> fast (`/fpfast`)
    - In Librarian->All Options, remove `/machine:X86 %(AdditionalOptions)`
    - In Librarian->General, select `Ignore All Default Libraries` -> `Yes`
    - Add `x64` as a target in the configuration manager, based on `Win32`

#### VS2013

1. Run `cmake -G "Visual Studio 12 2013" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF -DGLFW_USE_HYBRID_HPG=ON .` in the root
2. Follow remaining points from `VS2015`

### MinGW

1. Run `cmake -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_USE_HYBRID_HPG=ON .` in the root
2. Run `mingw32-make`
3. On a `Linux` machine (or find `binutils` for `Windows`), copy `libglfw3.a` + `libopengl32.a` & `libgdi32.a` (from MinGW installation) to a directory
4. Run `ar xv libopengl32.a | cut -f3 -d ' ' | xargs ar rvs libglfw3.a && rm *.o && echo 'done'`
5. Run `ar xv libgdi32.a | cut -f3 -d ' ' | xargs ar rvs libglfw3.a && rm *.o && echo 'done'`
6. Copy `libglfw3.a` back to the `Windows` machine
7. Run `ranlib libglfw3.a`
