#include "antpch.h"
#include "Ant/Renderer/RendererCommand.h"

namespace Ant {
	Scope<RendererAPI> RendererCommand::s_RendererAPI = RendererAPI::Create();

}
