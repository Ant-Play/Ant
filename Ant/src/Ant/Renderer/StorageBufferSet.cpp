#include "antpch.h"
#include "StorageBufferSet.h"

#include "UniformBufferSet.h"

#include "Renderer.h"

#include "Ant/Platform/Vulkan/VulkanStorageBufferSet.h"
#include "RendererAPI.h"

namespace Ant {

	Ref<StorageBufferSet> StorageBufferSet::Create(uint32_t frames)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:     return nullptr;
			case RendererAPIType::Vulkan:  return Ref<VulkanStorageBufferSet>::Create(frames);
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}