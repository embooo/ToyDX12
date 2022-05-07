-- https://premake.github.io/docs/Tokens/

workspace "ToyEngine"
	configurations { "Debug", "Release" }
	architecture "x64"
	startproject "ToyDX12"
	
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" -- ex: Debug-Windows-x64

	IncludePaths = {}
	IncludePaths["ToyDX12"]   = "ToyDX12/include/"

	SrcPaths = {}	  
	SrcPaths["ToyDX12"] = "ToyDX12/src/"

	BuildPaths = {}	  
	BuildPaths["ToyDX12"] = "ToyDX12/build/"
	
project "ToyDX12"
	location "ToyDX12"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	--pchheader "PCH.h"
	--pchsource (srcdir .. "PCH.cpp")

	files 
	{ 
		IncludePaths.ToyDX12 .. "**.h",
		SrcPaths.ToyDX12 .. "**.cpp"
	}

	includedirs
	{
		IncludePaths.ToyEngine
	}

	links
	{
	}

	defines
	{
	}
	
	targetdir	("ToyDX12/build/bin/" .. outputdir )
	objdir		("ToyDX12/build/obj/" .. outputdir )

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		buildoptions {"/Od"}
		defines { "DEBUG_BUILD" }

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		defines { "RELEASE_BUILD" }

-- Clean Function --
newaction {
	trigger     = "clean",
	description = "Removes generated files",
	execute     = function ()
	   print("Cleaning build folder...")
	   os.remove(BuildPaths.ToyDX12 .. "/**")
	   print("Done.")
	   print("Cleaning generated project files...")
	   os.remove("ToyEngine.sln")
	   os.remove("ToyDX12/*.vcxproj*")
	   os.rmdir(".vs")
	   print("Done.")
	end
 }