#include "antpch.h"
#include "Ant/Renderer/GraphicContext.h"

#include "Ant/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"


namespace Ant {

	Ant::Scope<Ant::GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				ANT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			}
			case RendererAPI::API::OpenGL:
			{
				return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
			}
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
