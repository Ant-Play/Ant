project "Ant"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "antpch.h"
    pchsource "src/antpch.cpp"

    files
    {
        "src/**.h",
        "src/**.cpp",
        "vendor/stb_image/**.h",
        "vendor/stb_image/**.cpp",
        "vendor/glm/glm/**.hpp",
        "vendor/glm/glm/**.inl",

        "vendor/ImGuizmo/ImGuizmo.h",
        "vendor/ImGuizmo/ImGuizmo.cpp",
        "/vendor/assimp/include"

    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE"
    }

    includedirs
    {
        "src",
        "vendor/spdlog/include",
        "vendor/assimp/include",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.mono}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.VulkanSDK}"
    }

    links
    {
        "Box2D",
        "GLFW",
        "Glad",
        "ImGui",
        "yaml-cpp",
        "opengl32.lib",

        "%{Library.mono}"
    }

    filter "files:vendor/ImGuizmo/**.cpp"
    flags { "NoPCH" }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "ANT_PLATFORM_WINDOWS",
        }

        links
        {
            "%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}",
        }

    filter "configurations:Debug"
        defines "ANT_DEBUG"
        runtime "Debug"
        symbols "on"

        links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

    filter "configurations:Release"
        defines "ANT_RELEASE"
        runtime "Release"
        optimize "on"

        links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

    filter "configurations:Dist"
        defines "ANT_DIST"
        runtime "Release"
        optimize "on"

        links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}