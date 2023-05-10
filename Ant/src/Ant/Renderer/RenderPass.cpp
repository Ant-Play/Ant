#include "antpch.h"
#include "Ant/Renderer/RenderPass.h"
#include "Ant/Renderer/Renderer.h"

#include "Ant/Platform/OpenGL/OpenGLRenderPass.h"

namespace Ant{
	
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None: ANT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPIType::OpenGL: return CreateRef<OpenGLRenderPass>(spec);
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}