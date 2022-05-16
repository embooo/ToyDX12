-- https://premake.github.io/docs/Tokens/

workspace "ToyEngine"
	configurations { "Debug", "Release" }
	architecture "x64"
	startproject "ToyDX12"
	
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" -- ex: Debug-Windows-x64

	FilePaths = {}
	FilePaths["Extern"]  = "ToyDX12/extern/"
	FilePaths["Src"]  = "ToyDX12/src/"
	FilePaths["Core"] =  FilePaths.Src .. "core/"
	FilePaths["App"]  =  FilePaths.Src .. "app/"
	FilePaths["GraphicsCore"]       =  FilePaths.Src .. "graphics/core/"
	FilePaths["GraphicsCore_DX12"]  =  FilePaths.Src .. "graphics/core/dx12/"
	FilePaths["GraphicsRendering"]  =  FilePaths.Src .. "graphics/rendering/"

	LibPaths = {}
	LibPaths["DirectX12"] = FilePaths.Extern .. "directx12"
	LibPaths["Spdlog"] = FilePaths.Extern .. "spdlog"


	BuildPaths = {}	  
	BuildPaths["ToyDX12"] = "ToyDX12/build/"
	
project "ToyDX12"
	location "ToyDX12"
	kind "WindowedApp" -- WinMain entry point
	language "C++"
	cppdialect "C++20"

	pchheader ("pch.h")
	pchsource (FilePaths.Core .. "pch.cpp")

	files 
	{ 
		FilePaths.Src .. "**.h",
		FilePaths.Src .. "**.cpp"
	}

	includedirs
	{
		FilePaths.Src .. "**",
		FilePaths.Extern
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