#include "antpch.h"
#include "Image.h"

#include "Ant/Platform/Vulkan/VulkanImage.h"
#include "Ant/Platform/OpenGL/OpenGLImage.h"

#include "RendererAPI.h"

namespace Ant{

	Ref<Image2D> Image2D::Create(ImageFormat format, uint32_t width, uint32_t height, Buffer buffer)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None: return nullptr;
			case RendererAPIType::OpenGL: return Ref<OpenGLImage2D>::Create(format, width, height, buffer);
			case RendererAPIType::Vulkan: return Ref<VulkanImage2D>::Create(format, width, height);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<Image2D> Image2D::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None: return nullptr;
			case RendererAPIType::OpenGL: return Ref<OpenGLImage2D>::Create(format, width, height, data);
			case RendererAPIType::Vulkan: return Ref<VulkanImage2D>::Create(format, width, height);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}