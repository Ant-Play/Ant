#include "antpch.h"
#include "StorageBuffer.h"

#include "Ant/Platform/Vulkan/VulkanStorageBuffer.h"
#include "Ant/Renderer/RendererAPI.h"

namespace Ant {

	Ref<StorageBuffer> StorageBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:     return nullptr;
			case RendererAPIType::Vulkan:  return Ref<VulkanStorageBuffer>::Create(size, binding);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}