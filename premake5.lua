-- https://premake.github.io/docs/Tokens/

workspace "ToyEngine"
	configurations { "Debug", "Release" }
	architecture "x64"
	startproject "ToyDX12"
	
	outputdir = "%{cfg.buildcfg}-%{prj.name}-%{cfg.system}-%{cfg.architecture}" -- ex: Debug-Windows-x64

	FilePaths = {}
	FilePaths["Extern"]  = "ToyDX12/extern/"
	FilePaths["Src"]  = "ToyDX12/src/"
	FilePaths["Core"] =  FilePaths.Src .. "core/"
	FilePaths["App"]  =  FilePaths.Src .. "app/"
	FilePaths["GraphicsCore"]       =  FilePaths.Src .. "graphics/core/"
	FilePaths["GraphicsCore_DX12"]  =  FilePaths.Src .. "graphics/core/dx12/"
	FilePaths["GraphicsRendering"]  =  FilePaths.Src .. "graphics/rendering/"
	FilePaths["Projects"]  =  "ToyDX12/projects/"


	LibPaths = {}
	LibPaths["DirectX12"] = FilePaths.Extern .. "directx12"
	LibPaths["Spdlog"] = FilePaths.Extern .. "spdlog"
	LibPaths["DirectXMath"] = FilePaths.Extern .. "directxmath"


	BuildPaths = {}	  
	BuildPaths["ToyDX12"] = "ToyDX12/build/"

------------------------------------------------------------------------------

-- Define projects 

projects = 
{
	"HelloApp"
}

-- Generate projects 

for i, name in ipairs(projects) do
	project(name) 
		location ("ToyDX12/projects/" .. name)
		kind "WindowedApp" -- WinMain entry point
		language "C++"
		cppdialect "C++20"

		pchheader ("pch.h")
		pchsource (FilePaths.Core .. "pch.cpp")

		files 
		{ 
			FilePaths.Src .. "**.h",
			FilePaths.Src .. "**.cpp",
			FilePaths.Projects .. name .. "/**.h",
			FilePaths.Projects .. name .. "/**.cpp",
		}

		includedirs
		{
			FilePaths.Src .. "**",
			FilePaths.Extern,
			LibPaths["DirectX12"],
			FilePaths.Projects .. name .. "/**",
			LibPaths["DirectXMath"]
		}

		links
		{
		"d3d12.lib",
		"dxgi.lib",
		"d3dcompiler.lib",
		"dxguid.lib"
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
end

------------------------------------------------------------------------------

-- Clean Function --
newaction 
{
	trigger     = "clean",
	description = "Removes generated files",
	execute     = function ()
	   print("Cleaning build folder...")
	   os.remove(BuildPaths.ToyDX12 .. "/**")
	   print("Done.")
	   print("Cleaning generated project files...")
	   os.remove("ToyEngine.sln")
	   os.remove("ToyDX12/**/*.vcxproj*")
	   os.rmdir(".vs")
	   print("Done.")
	end
 }