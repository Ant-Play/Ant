#include "antpch.h"
#include "Framebuffer.h"

#include "Ant/Platform/OpenGL/OpenGLFramebuffer.h"
#include "Ant/Platform/Vulkan/VulkanFramebuffer.h"

#include "RendererAPI.h"

namespace Ant {


	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		Ref<Framebuffer> result = nullptr;

		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:		return nullptr;
			//case RendererAPIType::OpenGL:	result = Ref<OpenGLFramebuffer>::Create(spec); break;
			case RendererAPIType::Vulkan:	result = Ref<VulkanFramebuffer>::Create(spec); break;
		}
		return result;
	}

}
