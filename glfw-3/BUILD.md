# Building GLFW3

## Windows

### Visual Studio

#### VS2015

1. Run `cmake -G "Visual Studio 14 2015" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF -DGLFW_USE_HYBRID_HPG=ON .` in the root directory
2. Open `GLFW.sln`
3. Edit `glfw`'s project properties
    - Floating point model -> fast (`/fpfast`)
    - In Librarian->All Options, remove `/machine:X86 %(AdditionalOptions)`
    - In Librarian->General, select `Ignore All Default Libraries` -> `Yes`
    - Add `x64` as a target in the configuration manager, based on `Win32`

#### VS2013

1. Run `cmake -G "Visual Studio 12 2013" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF -DGLFW_USE_HYBRID_HPG=ON .` in the root directory
2. Follow remaining points from `VS2015`

### MinGW

1. Run `cmake -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_USE_HYBRID_HPG=ON .` in the root directory
2. Run `mingw32-make`
3. On a `Linux` machine (or using `binutils` for `Windows`), copy `libglfw3.a` + `libopengl32.a` & `libgdi32.a` (from MinGW installation) to the same directory
4. Run `ar xv libopengl32.a | cut -f3 -d ' ' | xargs ar rvs libglfw3.a && rm *.o && echo 'done'`
5. Run `ar xv libgdi32.a | cut -f3 -d ' ' | xargs ar rvs libglfw3.a && rm *.o && echo 'done'`
6. Copy `libglfw3.a` back to the `Windows` machine
7. Run `ranlib libglfw3.a`

## Linux

1. Run `cmake -G "Unix Makefiles" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF .` in the root directory
2. Run `make`

## OSX

1. Add `set(CMAKE_OSX_ARCHITECTURES i386;x86_64)` to the `Cocoa` section of `CMakeLists.txt`
3. Add `set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)` to the `Cocoa` section of `CMakeLists.txt`
3. Run `cmake -G "Xcode" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF .` in the root directory
4. Open `GLFW.xcodeproj`
5. Switch scheme to `Release`
6. Build
