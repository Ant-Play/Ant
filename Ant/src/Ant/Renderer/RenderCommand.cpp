#include "antpch.h"
#include "Ant/Renderer/RenderCommand.h"

namespace Ant {
	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

}
