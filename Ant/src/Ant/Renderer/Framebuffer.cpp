#include "antpch.h"
#include "Ant/Renderer/Framebuffer.h"
#include "Ant/Renderer/Renderer.h"

#include "Ant/Platform/OpenGL/OpenGLFramebuffer.h"


namespace Ant {


	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    ANT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLFramebuffer>(spec);
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
