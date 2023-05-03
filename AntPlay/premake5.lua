project "AntPlay"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

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
		"%{IncludeDir.entt}",
		--"%{IncludeDir.filewatch}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGuizmo}"
	}

	links
    {
        "Ant"
    }

	filter "system:windows"
        systemversion "latest"
    
    filter "configurations:Debug"
        defines "ANT_DEBUG"
        runtime "Debug"
        symbols "on"
    
    filter "configurations:Release"
        defines "ANT_RELEASE"
        runtime "Release"
        optimize "on"
    
    filter "configurations:Dist"
        defines "ANT_DIST"
        runtime "Release"
        optimize "on"
