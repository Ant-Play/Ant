include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Ant"
    configurations { "Debug", "Release", "Dist" }
        targetdir "build"
        startproject "AntPlay"

    solution_items
	{
		".editorconfig"
	}

	configurations
	{ 
		"Debug", 
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	filter "language:C++ or language:C"
		architecture "x86_64"
	filter ""

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
include "Ant/vendor/GLFW"
include "Ant/vendor/Glad"
include "Ant/vendor/imgui"
include "Ant/vendor/Box2D"
group ""

group "Core"
project "Ant"
    location "Ant"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "antpch.h"
	pchsource "Ant/src/antpch.cpp"

    files 
	{ 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp",

		"%{prj.name}/vendor/stb_image/**.h",
        "%{prj.name}/vendor/stb_image/**.cpp",

		"%{prj.name}/vendor/FastNoise/**.cpp",

		"%{prj.name}/vendor/yaml-cpp/src/**.cpp",
		"%{prj.name}/vendor/yaml-cpp/src/**.h",
		"%{prj.name}/vendor/yaml-cpp/include/**.h",
		"%{prj.name}/vendor/VulkanMemoryAllocator/**.h",
		"%{prj.name}/vendor/VulkanMemoryAllocator/**.cpp",

		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
        "%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp",

		"%{prj.name}/vendor/imgui/examples/imgui_impl_vulkan_with_textures.h",
        "%{prj.name}/vendor/imgui/examples/imgui_impl_vulkan_with_textures.cpp",
	}

    includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor",

		"%{IncludeDir.Assimp}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.FastNoise}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/PhysX",
		"%{IncludeDir.VulkanSDK}",
	}

    links
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"Box2D",
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

    filter "files:Ant/vendor/FastNoise/**.cpp or files:Ant/vendor/yaml-cpp/src/**.cpp or files:Ant/vendor/ImGuizmo/**.cpp or Ant/vendor/imgui/examples/imgui_impl_vulkan_with_textures.cpp"
	flags { "NoPCH" }

    filter "system:windows"
		systemversion "latest"

        defines 
		{ 
			"ANT_PLATFORM_WINDOWS",
			"ANT_BUILD_DLL"
		}

		links
        {
            "%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}",
        }

    filter "configurations:Debug"
		symbols "On"

        includedirs
		{
			"%{IncludeDir.NvidiaAftermath}",
			"%{IncludeDir.shaderc_util}",
			"%{IncludeDir.shaderc_glslc}"
		}

        links
		{
			"%{Library.NvidiaAftermath}",

			"%{Library.ShaderC_Debug}",
			"%{Library.ShaderC_Utils_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}",
			"%{Library.SPIRV_Tools_Debug}",
		}

        defines 
		{
			"ANT_DEBUG",
			"ANT_TRACK_MEMORY"
		}

    filter "configurations:Release"
		optimize "On"

        includedirs
		{
			"%{IncludeDir.NvidiaAftermath}",
			"%{IncludeDir.shaderc_util}",
			"%{IncludeDir.shaderc_glslc}"
		}

        defines
		{
			"ANT_RELEASE",
			"ANT_TRACK_MEMORY",
			"NDEBUG" -- PhysX Requires This
		}

        links
		{
			"%{Library.NvidiaAftermath}",

			"%{Library.ShaderC_Release}",
			"%{Library.ShaderC_Utils_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
		}

    filter "configurations:Dist"
		optimize "On"
		symbols "Off"

        defines
		{
			"ANT_DIST",
			"NDEBUG" -- PhysX Requires This
		}

        removefiles
		{
			"%{prj.name}/src/Ant/Platform/Vulkan/ShaderCompiler/**.cpp",
			"%{prj.name}/src/Ant/Platform/Vulkan/Debug/**.cpp",

			"%{prj.name}/src/Ant/Animation/AnimationImporterAssimp.cpp",
			"%{prj.name}/src/Ant/Asset/AssimpMeshImporter.cpp",
		}

project "Hazel-ScriptCore"
	location "Ant-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
	{
		"%{prj.name}/src/**.cs",
		"%{prj.name}/Properties/**.cs"
	}

    filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"

group ""

group "Tools"
project "AntPlay"
	location "AntPlay"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    links 
	{ 
		"Ant"
	}

    defines 
	{
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
	}

    files 
	{ 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp", 
		
		-- Shaders
		--"%{prj.name}/Resources/Shaders/**.glsl", 
		--"%{prj.name}/Resources/Shaders/**.glslh", 
		--"%{prj.name}/Resources/Shaders/**.hlsl", 
		--"%{prj.name}/Resources/Shaders/**.hlslh", 
		--"%{prj.name}/Resources/Shaders/**.slh", 
	}

    includedirs 
	{
		"%{prj.name}/src",
		"Ant/src",
		"Ant/vendor",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/PhysX",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.Glad}"
	}

    postbuildcommands 
	{
		'{COPY} "../Ant/vendor/NvidiaAftermath/lib/x64/GFSDK_Aftermath_Lib.x64.dll" "%{cfg.targetdir}"'
	}

    filter "system:windows"
		systemversion "latest"

        defines 
		{ 
			"Ant_PLATFORM_WINDOWS"
		}

		links
        {
            "%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}",
        }

    filter "configurations:Debug"
		symbols "on"

        includedirs
		{
			--"%{IncludeDir.Optick}",
		}

        defines
		{
			"Ant_DEBUG",
			"Ant_TRACK_MEMORY"
		}

        links
		{
			"%{Library.Assimp_Debug}"
		}

        postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Debug}" "%{cfg.targetdir}"',
		}

    filter "configurations:Release"
		optimize "on"

        includedirs
		{
			--"%{IncludeDir.Optick}",
		}

        defines
		{
			"ANT_RELEASE",
			"ANT_TRACK_MEMORY",
			"NDEBUG" -- PhysX Requires This
		}

        links
		{
			"%{Library.Assimp_Release}"
		}

        postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
		}

    filter "configurations:Dist"
		optimize "on"

        defines
		{
			"ANT_DIST",
			"NDEBUG" -- PhysX Requires This
		}

        links
		{
			"%{Library.Assimp_Release}"
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
		}
group ""

workspace "Sandbox"
	architecture "x64"
	targetdir "build"
	
	configurations 
	{ 
		"Debug", 
		"Release",
		"Dist"
	}

project "Ant-ScriptCore"
	location "Ant-ScriptCore"
	kind "SharedLib"
	language "C#"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.cs", 
	}

project "ExampleApp"
	location "ExampleApp"
	kind "SharedLib"
	language "C#"

	targetdir ("AntPlay/assets/scripts")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.cs", 
	}

	links
	{
		"Ant-ScriptCore"
	}
group ""