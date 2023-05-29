
-- Ant Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["Assimp"] = "%{wks.location}/Ant/vendor/assimp/include"
IncludeDir["stb_image"] = "%{wks.location}/Ant/vendor/stb_image"
IncludeDir["yaml_cpp"] = "%{wks.location}/Ant/vendor/yaml-cpp/include"
IncludeDir["GLFW"] = "%{wks.location}/Ant/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Ant/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Ant/vendor/imgui"
IncludeDir["ImGuiNodeEditor"] = "%{wks.location}/Ant/vendor/imgui-node-editor"
IncludeDir["glm"] = "%{wks.location}/Ant/vendor/glm"
IncludeDir["Box2D"] = "%{wks.location}/Ant/vendor/Box2D/include"
IncludeDir["entt"] = "%{wks.location}/Ant/vendor/entt/include"
IncludeDir["FastNoise"] = "%{wks.location}/Ant/vendor/FastNoise"
IncludeDir["mono"] = "%{wks.location}/Ant/vendor/mono/include"
IncludeDir["PhysX"] = "%{wks.location}/Ant/vendor/PhysX/include"
IncludeDir["shaderc_util"] = "%{wks.location}/Ant/vendor/shaderc/libshaderc_util/include"
IncludeDir["shaderc"] = "%{wks.location}/Ant/vendor/shaderc/include"
IncludeDir["SPIRV_Cross"] = "%{wks.location}/Ant/vendor/SPIRV-Cross"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["NvidiaAftermath"] = "%{wks.location}/Ant/vendor/NvidiaAftermath/include"
IncludeDir["miniaudio"] = "%{wks.location}/Ant/vendor/miniaudio/include"
IncludeDir["farbot"] = "%{wks.location}/Ant/vendor/farbot/include"
IncludeDir["Optick"] = "%{wks.location}/Ant/vendor/Optick/src"
IncludeDir["msdf_atlas_gen"] = "%{wks.location}/Ant/vendor/msdf-atlas-gen/msdf-atlas-gen"
IncludeDir["msdfgen"] = "%{wks.location}/Ant/vendor/msdf-atlas-gen/msdfgen"
IncludeDir["choc"] = "%{wks.location}/Ant/vendor/choc"
IncludeDir["magic_enum"] = "%{wks.location}/Ant/vendor/magic_enum/include"

LibraryDir = {}

LibraryDir["PhysX"] = "%{wks.location}/Ant/vendor/PhysX/lib/%{cfg.buildcfg}"
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["NvidiaAftermath"] = "%{wks.location}/Ant/vendor/NvidiaAftermath/lib/x64"
LibraryDir["Mono"] = "%{wks.location}/Ant/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["Assimp_Debug"] = "%{wks.location}/Ant/vendor/assimp/bin/Debug/assimp-vc143-mtd.lib"
Library["Assimp_Release"] = "%{wks.location}/Ant/vendor/assimp/bin/Release/assimp-vc143-mt.lib"
Library["mono"] = "%{LibraryDir.Mono}/mono-2.0-sgen.lib"

Library["PhysX"] = "%{LibraryDir.PhysX}/PhysX_static_64.lib"
Library["PhysXCharacterKinematic"] = "%{LibraryDir.PhysX}/PhysXCharacterKinematic_static_64.lib"
Library["PhysXCommon"] = "%{LibraryDir.PhysX}/PhysXCommon_static_64.lib"
Library["PhysXCooking"] = "%{LibraryDir.PhysX}/PhysXCooking_static_64.lib"
Library["PhysXExtensions"] = "%{LibraryDir.PhysX}/PhysXExtensions_static_64.lib"
Library["PhysXFoundation"] = "%{LibraryDir.PhysX}/PhysXFoundation_static_64.lib"
Library["PhysXPvd"] = "%{LibraryDir.PhysX}/PhysXPvdSDK_static_64.lib"

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["NvidiaAftermath"] = "%{LibraryDir.NvidiaAftermath}/GFSDK_Aftermath_Lib.x64.lib"
Library["dxc"] = "%{LibraryDir.VulkanSDK}/dxcompiler.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["ShaderC_Utils_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_utild.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["ShaderC_Utils_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_util.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Temporarily use our own SPIRV-Cross binaries
-- Vulkan SDK 1.3.204.1 (currently latest) shipped with a critical bug for us (see https://github.com/KhronosGroup/SPIRV-Cross/issues/1879)
Library["SPIRV_Cross_Debug"] = "%{wks.location}/Ant/vendor/SPIRV-Cross/lib/spirv-cross-cored.lib"
Library["SPIRV_Cross_Release"] = "%{wks.location}/Ant/vendor/SPIRV-Cross/lib/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{wks.location}/Ant/vendor/SPIRV-Cross/lib/spirv-cross-glsld.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{wks.location}/Ant/vendor/SPIRV-Cross/lib/spirv-cross-glsl.lib"

-- Windows-only
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"

Binaries = {}
Binaries["Assimp_Debug"] = "%{wks.location}/Ant/vendor/assimp/bin/Debug/assimp-vc143-mtd.dll"
Binaries["Assimp_Release"] = "%{wks.location}/Ant/vendor/assimp/bin/Release/assimp-vc143-mt.dll"