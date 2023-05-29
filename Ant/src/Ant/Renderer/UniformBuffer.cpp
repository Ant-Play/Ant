#include "antpch.h"
#include "UniformBuffer.h"

#include "Ant/Renderer/Renderer.h"

#include "Ant/Platform/Vulkan/VulkanUniformBuffer.h"

#include "Ant/Renderer/RendererAPI.h"

namespace Ant {

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:     return nullptr;
			case RendererAPIType::Vulkan:  return Ref<VulkanUniformBuffer>::Create(size, binding);
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}