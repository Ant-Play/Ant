#include "antpch.h"
#include "Ant/Renderer/RendererCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Ant {
	RendererAPI* RendererCommand::s_RendererAPI = new OpenGLRendererAPI;

}
