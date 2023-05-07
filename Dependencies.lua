
-- Ant Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Ant/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Ant/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Ant/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Ant/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/Ant/vendor/stb_image"
IncludeDir["entt"] = "%{wks.location}/Ant/vendor/entt/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Ant/vendor/yaml-cpp/include"
IncludeDir["Box2D"] = "%{wks.location}/Ant/vendor/Box2D/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Ant/vendor/ImGuizmo"
IncludeDir["entt"] = "%{wks.location}/Ant/vendor/entt/include"
IncludeDir["mono"] = "%{wks.location}/Ant/vendor/mono/include"
IncludeDir["shaderc"] = "%{wks.location}/Ant/vendor/shaderc/include"
IncludeDir["SPIRV_Cross"] = "%{wks.location}/Ant/vendor/SPIRV-Cross"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["mono"] = "%{wks.location}/Ant/vendor/mono/lib/%{cfg.buildcfg}"

-- mono now use static library
Library = {}
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Windows-only
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"