project "Ant"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "antpch.h"
    pchsource "src/antpch.cpp"

    files
    {
        "src/**.h", 
		"src/**.c", 
		"src/**.hpp", 
		"src/**.cpp",

        "vendor/stb_image/**.h",
        "vendor/stb_image/**.cpp",
        "vendor/glm/glm/**.hpp",
        "vendor/glm/glm/**.inl",

        "vendor/ImGuizmo/ImGuizmo.h",
        "vendor/ImGuizmo/ImGuizmo.cpp",
        "vendor/assimp/include",

        "vendor/FastNoise/Cpp/**.h",

        "vendor/VulkanMemoryAllocator/**.h",
		"vendor/VulkanMemoryAllocator/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/Ant/src",
        "%{wks.location}/Ant/vendor",
        
        "%{IncludeDir.Assimp}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.mono}",
        "%{IncludeDir.FastNoise}",
        "%{IncludeDir.PhysX}",
        "%{IncludeDir.PhysX}/PhysX",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.VulkanSDK}",
    }
    
    links
    {
        "Box2D",
        "GLFW",
        "Glad",
        "ImGui",
        "yaml-cpp",
        "opengl32.lib",
        
        "%{Library.Vulkan}",
		"%{Library.VulkanUtils}",
        
        "%{Library.mono}",
        "%{Library.PhysX}",
		"%{Library.PhysXCharacterKinematic}",
		"%{Library.PhysXCommon}",
		"%{Library.PhysXCooking}",
		"%{Library.PhysXExtensions}",
		"%{Library.PhysXFoundation}",
		"%{Library.PhysXPvd}",
    }
    
    defines
    {
        "PX_PHYSX_STATIC_LIB", "GLM_FORCE_DEPTH_ZERO_TO_ONE"
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
        symbols "On"

        links
		{
            "%{Library.NvidiaAftermath}",

			"%{Library.ShaderC_Debug}",
            "%{Library.ShaderC_Utils_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}",
            "%{Library.SPIRV_Tools_Debug}",
		}

        includedirs
        {
            "%{IncludeDir.NvidiaAftermath}",
            "%{IncludeDir.shaderc_util}",
			"%{IncludeDir.shaderc_glslc}"
        }

        defines 
		{
			"ANT_DEBUG",
			"ANT_TRACK_MEMORY"
		}

    filter "configurations:Release"
        defines "ANT_RELEASE"
        optimize "On"

        includedirs
        {
            "%{IncludeDir.NvidiaAftermath}",
            "%{IncludeDir.shaderc_util}",
			"%{IncludeDir.shaderc_glslc}"
        }

        links
		{
            "%{Library.NvidiaAftermath}",
			"%{Library.ShaderC_Release}",
            "%{Library.ShaderC_Utils_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

        defines
		{
			"ANT_RELEASE",
			"ANT_TRACK_MEMORY",
			"NDEBUG" -- PhysX Requires This
		}

    filter "configurations:Dist"
        defines "ANT_DIST"
        optimize "On"

        includedirs
        {
            "%{IncludeDir.NvidiaAftermath}",
        }

        defines
		{
			"ANT_DIST",
			"NDEBUG" -- PhysX Requires This
		}

        links
		{

            "%{Library.NvidiaAftermath}",
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}