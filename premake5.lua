workspace "Ant"
    architecture "x64"
    startproject "AntPlay"

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
IncludeDir["glm"] = "Ant/vendor/glm"
IncludeDir["stb_image"] = "Ant/vendor/stb_image"

group "Dependencies"
    include "Ant/vendor/GLFW"
    include "Ant/vendor/Glad"
    include "Ant/vendor/imgui"
group ""


project "Ant"
    location "Ant"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "antpch.h"
    pchsource "Ant/src/antpch.cpp"
    
    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/stb_image/**.h",
        "%{prj.name}/vendor/stb_image/**.cpp",
        "%{prj.name}/vendor/glm/glm/**.hpp",
        "%{prj.name}/vendor/glm/glm/**.inl"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}"
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
        "opengl32.lib"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "ANT_PLATFORM_WINDOWS",
            "ANT_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }

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

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

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
        "Ant/vendor",
        "%{IncludeDir.glm}"
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


project "AntPlay"
    location "AntPlay"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    
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
        "Ant/vendor",
        "%{IncludeDir.glm}"
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


