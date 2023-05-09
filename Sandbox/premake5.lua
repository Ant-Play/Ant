project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Ant/vendor/spdlog/include",
		"%{wks.location}/Ant/src",
		"%{wks.location}/Ant/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
	}

	links
	{
		"Ant"
	}

	filter "system:windows"
		systemversion "latest"
		defines 
		{ 
			"ANT_PLATFORM_WINDOWS"
		}


	filter "configurations:Debug"
		defines "ANT_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{wks.location}/Ant/vendor/assimp/bin/Debug/assimp-vc143-mtd.lib"
		}

	filter "configurations:Release"
		defines "ANT_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{wks.location}/Ant/vendor/assimp/bin/Release/assimp-vc143-mt.lib"
		}

	filter "configurations:Dist"
		defines "ANT_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{wks.location}/Ant/vendor/assimp/bin/Release/assimp-vc143-mt.lib"
		}
