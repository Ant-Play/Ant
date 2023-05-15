#include "antpch.h"
#include "RendererContext.h"

#include "RendererAPI.h"

#include "Ant/Platform/OpenGL/OpenGLContext.h"
#include "Ant/Platform/Vulkan/VulkanContext.h"

namespace Ant{

	Ref<RendererContext> RendererContext::Create(GLFWwindow* windowHandle)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:    return nullptr;
			case RendererAPIType::OpenGL:  return Ref<OpenGLContext>::Create(windowHandle);
			case RendererAPIType::Vulkan:  return Ref<VulkanContext>::Create(windowHandle);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
