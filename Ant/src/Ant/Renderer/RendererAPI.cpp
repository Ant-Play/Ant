#include "antpch.h"
#include "Ant/Renderer/RendererAPI.h"
#include "Ant/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"


namespace Ant {

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	Ant::Scope<Ant::RendererAPI> RendererAPI::Create()
	{
		switch (Renderer::GetAPI())
		{
			case API::None:
			{
				ANT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			}
			case API::OpenGL:
			{
				return CreateScope<OpenGLRendererAPI>();
			}
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
