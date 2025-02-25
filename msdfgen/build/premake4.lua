function initconfigurations ()
    return
    {
        "Debug",
        "Release"
    }
end

function initplatforms ()
    if os.is ("windows")
    or os.is ("linux") then
        if os.is64bit () then
            return
            {
                "x64",
                "x32"
            }
        else
            return
            {
                "x32",
                "x64"
            }
        end
    elseif os.is ("macosx") then
        return
        {
            "x64"
        }
    end
end

function defaultaction (name, action)
   if os.is (name) then
      _ACTION = _ACTION or action
   end
end

defaultaction ("windows", "vs2022")
defaultaction ("linux", "gmake")
defaultaction ("macosx", "gmake")

if os.is ("macosx") then
    osname = "mac"
else
    osname = os.get()
end

destination = "./" .. osname .. "/" .. _ACTION

--
-- Solution: msdfgen
--

solution "msdfgen"

    language ("C++")

    location (destination)

    kind ("StaticLib")

    configurations
    {
        initconfigurations ()
    }

    platforms
    {
        initplatforms ()
    }

    flags
    {
        "NoPCH",
        "NoManifest",
        "FloatFast",
        "NoNativeWChar",
        "NoExceptions",
        "NoIncrementalLink",
        "NoEditAndContinue",
        "NoMinimalRebuild",
        "Symbols",
        "StaticRuntime"
    }



    configuration {"linux", "x32"}
        targetdir ("../lib/linux/32")

    configuration {"linux", "x64"}
        targetdir ("../lib/linux/64")

    configuration {"macosx"}
        targetdir ("../lib/mac")

    configuration {"windows", "gmake or codelite or codeblocks", "x32"}
        targetdir ("../lib/mingw/32")

    configuration {"windows", "gmake or codelite or codeblocks", "x64"}
        targetdir ("../lib/mingw/64")

    configuration {"vs2015 or vs2017 or vs2019 or vs2022", "x32"}
        targetdir ("../lib/vc2015/32")

    configuration {"vs2015 or vs2017 or vs2019 or vs2022", "x64"}
        targetdir ("../lib/vc2015/64")

    configuration {"x32"}
        flags {"EnableSSE2"}

    configuration {"not windows"}
        flags {"Unicode"}

    configuration {"*Debug*"}
        targetsuffix ("d")

    configuration {"*Release*"}
        flags {"Optimize", "NoRTTI"}


-- Linux

    configuration {"linux"}
        buildoptions
        {
            "-fPIC",
            "-std=c++11"
        }
        links
        {
            "dl",
            "m",
            "rt"
        }


-- Mac OS X

    configuration {"macosx"}
        buildoptions
        {
            "-stdlib=libc++",
            "-std=c++11",
            "-gdwarf-2"
        }


-- Windows

    configuration {"windows", "vs*"}
        buildoptions
        {
            "/MP"
        }

    configuration {"windows", "gmake", "x32"}
        prebuildcommands
        {
            "$(eval CC := i686-w64-mingw32-gcc)",
            "$(eval CXX := i686-w64-mingw32-g++)",
            "$(eval AR := i686-w64-mingw32-gcc-ar)"
        }

    configuration {"windows", "gmake", "x64"}
        prebuildcommands
        {
            "$(eval CC := x86_64-w64-mingw32-gcc)",
            "$(eval CXX := x86_64-w64-mingw32-g++)",
            "$(eval AR := x86_64-w64-mingw32-gcc-ar)"
        }

    configuration {"windows", "codelite or codeblocks", "x32"}
        envs
        {
            "CC=i686-w64-mingw32-gcc",
            "CXX=i686-w64-mingw32-g++",
            "AR=i686-w64-mingw32-gcc-ar"
        }

    configuration {"windows", "codelite or codeblocks", "x64"}
        envs
        {
            "CC=x86_64-w64-mingw32-gcc",
            "CXX=x86_64-w64-mingw32-g++",
            "AR=x86_64-w64-mingw32-gcc-ar"
        }


--
-- Project: msdfgen
--

project "msdfgen"

    files
    {
        "../**.cpp",
        "../**.h"
    }

    includedirs
    {
        "../include"
    }

    defines
    {
      "MSDFGEN_NO_FREETYPE=1"
    }

-- Windows

    configuration {"windows", "debug", "vs*"}
        defines
        {
            "_ITERATOR_DEBUG_LEVEL=2"
        }
