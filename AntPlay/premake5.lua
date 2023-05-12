project "AntPlay"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.c",
		"src/**.hpp",
		"src/**.cpp"
	}

	includedirs
	{
		"/src",
		"%{wks.location}/Ant/vendor/spdlog/include",
		"%{wks.location}/Ant/src",
		"%{wks.location}/Ant/vendor",
		"%{IncludeDir.entt}",
		--"%{IncludeDir.filewatch}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGuizmo}"
	}

	links
    {
        "Ant"
    }

    postbuildcommands 
	{
		-- '{COPY} "%{wks.location}/AntPlay/assets" "%{cfg.targetdir}/assets"'
	}

	filter "system:windows"
        systemversion "latest"

        defines 
		{ 
			"ANT_PLATFORM_WINDOWS"
		}
    
    filter "configurations:Debug"
        defines "ANT_DEBUG"
        symbols "on"

        links
		{
			"%{wks.location}/Ant/vendor/assimp/bin/Debug/assimp-vc143-mtd.lib"
		}

        postbuildcommands 
		{
			'{COPY} "%{wks.location}/Ant/vendor/assimp/bin/Debug/assimp-vc143-mtd.dll" "%{cfg.targetdir}"'
		}
    
    filter "configurations:Release"
        defines "ANT_RELEASE"
        optimize "on"

        links
		{
			"%{wks.location}/Ant/vendor/assimp/bin/Release/assimp-vc143-mt.lib"
		}

		postbuildcommands 
		{
			'{COPY} "%{wks.location}/Ant/vendor/assimp/bin/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"'
		}
    
    filter "configurations:Dist"
        defines "ANT_DIST"
        optimize "on"

        links
		{
			"%{wks.location}/Ant/vendor/assimp/bin/Release/assimp-vc143-mt.lib"
		}

		postbuildcommands 
		{
			'{COPY} "%{wks.location}/Ant/vendor/assimp/bin/Release/assimp-vc143-mtd.dll" "%{cfg.targetdir}"'
		}
