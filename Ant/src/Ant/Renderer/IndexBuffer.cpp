#include "antpch.h"
#include "IndexBuffer.h"

#include "Renderer.h"

#include "Ant/Platform/OpenGL/OpenGLIndexBuffer.h"
#include "Ant/Platform/Vulkan/VulkanIndexBuffer.h"

#include "RendererAPI.h"

namespace Ant{

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:    return nullptr;
			case RendererAPIType::OpenGL:  return Ref<OpenGLIndexBuffer>::Create(size);
			case RendererAPIType::Vulkan:  return Ref<VulkanIndexBuffer>::Create(size);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(void* data, uint32_t size)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:    return nullptr;
			case RendererAPIType::OpenGL:  return Ref<OpenGLIndexBuffer>::Create(data, size);
			case RendererAPIType::Vulkan:  return Ref<VulkanIndexBuffer>::Create(data, size);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
