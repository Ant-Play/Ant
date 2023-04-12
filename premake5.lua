workspace "Ant"
    architecture "x64"
    startproject "Sandbox"

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
IncludeDir["Glad"] = "Ant/vendor/Glad/include"
IncludeDir["ImGui"] = "Ant/vendor/imgui"

include "Ant/vendor/GLFW"
include "Ant/vendor/Glad"
include "Ant/vendor/imgui"

--startproject "Sandbox"

project "Ant"
    location "Ant"
    kind "SharedLib"
    language "C++"
    staticruntime "off"

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
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}"
        --"Ant/src",
        --"Ant/vendor/spdlog/include",
        --"Ant/vendor/GLFW/include"
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
        "opengl32.lib"
    }

    filter "system:windows"
        cppdialect "C++17"
        systemversion "latest"

        defines
        {
            "ANT_PLATFORM_WINDOWS",
            "ANT_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }

        postbuildcommands
        {
            -- ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
            ("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
        }

    filter "configurations:Debug"
        defines "ANT_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "ANT_RELEASE"
        runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        defines "ANT_DIST"
        runtime "Release"
        optimize "On"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    staticruntime "off"

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
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}"
    }

    links
    {
        "Ant"
    }

    filter "system:windows"
        cppdialect "C++17"
        systemversion "latest"

        defines
        {
            "ANT_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "ANT_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "ANT_RELEASE"
        runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        defines "ANT_DIST"
        runtime "Release"
        optimize "On"


