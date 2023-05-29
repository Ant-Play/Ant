#include "antpch.h"
#include "Material.h"

#include "Ant/Platform/Vulkan/VulkanMaterial.h"
#include "Ant/Platform/OpenGL/OpenGLMaterial.h"

#include "RendererAPI.h"

namespace Ant{

	Ref<Material> Material::Create(const Ref<Shader>& shader, const std::string& name)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None: return nullptr;
			case RendererAPIType::Vulkan: return Ref<VulkanMaterial>::Create(shader, name);
			//case RendererAPIType::OpenGL: return Ref<OpenGLMaterial>::Create(shader, name);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<Material> Material::Copy(const Ref<Material>& other, const std::string& name)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None: return nullptr;
			case RendererAPIType::Vulkan: return Ref<VulkanMaterial>::Create(other, name);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}