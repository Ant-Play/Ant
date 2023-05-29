#include "antpch.h"
#include "UniformBufferSet.h"

#include "Ant/Renderer/Renderer.h"

#include "Ant/Platform/Vulkan/VulkanUniformBufferSet.h"

#include "Ant/Renderer/RendererAPI.h"

namespace Ant {

	Ref<UniformBufferSet> UniformBufferSet::Create(uint32_t frames)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:     return nullptr;
			case RendererAPIType::Vulkan:  return Ref<VulkanUniformBufferSet>::Create(frames);
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}