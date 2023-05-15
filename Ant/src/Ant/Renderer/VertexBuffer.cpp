#include "antpch.h"
#include "VertexBuffer.h"

#include "Renderer.h"

#include "Ant/Platform/OpenGL/OpenGLVertexBuffer.h"
#include "Ant/Platform/Vulkan/VulkanVertexBuffer.h"

#include "RendererAPI.h"

namespace Ant{

	Ref<VertexBuffer> VertexBuffer::Create(void* data, uint32_t size, VertexBufferUsage usage)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:    return nullptr;
			case RendererAPIType::OpenGL:  return Ref<OpenGLVertexBuffer>::Create(data, size, usage);
			case RendererAPIType::Vulkan:  return Ref<VulkanVertexBuffer>::Create(data, size, usage);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, VertexBufferUsage usage)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:    return nullptr;
			case RendererAPIType::OpenGL:  return Ref<OpenGLVertexBuffer>::Create(size, usage);
			case RendererAPIType::Vulkan:  return Ref<VulkanVertexBuffer>::Create(size, usage);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
