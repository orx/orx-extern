--
-- vs2022.lua
-- Baseline support for Visual Studio 2022.
-- Copyright (c) 2013 Jason Perkins and the Premake project
--

	premake.vstudio.vc2022 = {}
	local vc2022 = premake.vstudio.vc2022
	local vstudio = premake.vstudio


---
-- Register a command-line action for Visual Studio 2022.
---

	newaction
	{
		trigger         = "vs2022",
		shortname       = "Visual Studio 2022",
		description     = "Generate Microsoft Visual Studio 2022 project files",
		os              = "windows",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },

		valid_languages = { "C", "C++", "C#"},

		valid_tools     = {
			cc     = { "msc"   },
			dotnet = { "msnet" },
		},

		onsolution = function(sln)
			premake.generate(sln, "%%.sln", vstudio.sln2005.generate)
		end,

		onproject = function(prj)
			if premake.isdotnetproject(prj) then
				premake.generate(prj, "%%.csproj", vstudio.cs2005.generate)
				premake.generate(prj, "%%.csproj.user", vstudio.cs2005.generate_user)
			else
				premake.generate(prj, "%%.vcxproj", premake.vs2010_vcxproj)
				premake.generate(prj, "%%.vcxproj.user", premake.vs2010_vcxproj_user)
				premake.generate(prj, "%%.vcxproj.filters", vstudio.vc2010.generate_filters)
			end
		end,


		oncleansolution = premake.vstudio.cleansolution,
		oncleanproject  = premake.vstudio.cleanproject,
		oncleantarget   = premake.vstudio.cleantarget,

		vstudio = {
			solutionVersion = "12",
			targetFramework = "4.8",
			targetPlatform  = "10.0",
			toolsVersion    = "17.0",
		}
	}
