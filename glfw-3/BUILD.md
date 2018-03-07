# Building GLFW3

## Windows

### Visual Studio

1. run `cmake -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF -DGLFW_USE_HYBRID_HPG=ON .` in the root
2. Open `GLFW.sln`
3. Edit `glfw`'s project properties
    - Floating point model -> fast (`/fpfast`)
    - In Librarian->All Options, remove `/machine:X86 %(AdditionalOptions)`
    - Add `x64` as a target in the configuration manager, based on `Win32`
