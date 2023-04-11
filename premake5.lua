workspace "Ant"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Ant/vendor/GLFW/include"

include "Ant/vendor/GLFW"

project "Ant"
    location "Ant"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "antpch.h"
    pchsource "Ant/src/antpch.cpp"
    
    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}"
        --"Ant/src",
        --"Ant/vendor/spdlog/include",
        --"Ant/vendor/GLFW/include"
    }

    links
    {
        "GLFW",
        "opengl32.lib"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "ANT_PLATFORM_WINDOWS",
            "ANT_BUILD_DLL"
        }

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
        }

    filter "configurations:Debug"
        defines "ANT_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "ANT_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "ANT_DIST"
        optimize "On"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs
    {
        "Ant/vendor/spdlog/include",
        "Ant/src",
        "%{IncludeDir.GLFW}"
    }

    links
    {
        "Ant"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "ANT_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "ANT_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "ANT_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "ANT_DIST"
        optimize "On"

