#include "antpch.h"
#include "RenderPass.h"

#include "Renderer.h"

#include "Ant/Platform/OpenGL/OpenGLRenderPass.h"
#include "Ant/Platform/Vulkan/VulkanRenderPass.h"

#include "RendererAPI.h"

namespace Ant{
	
	Ant::Ref<Ant::RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:    ANT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPIType::Vulkan:  return Ref<VulkanRenderPass>::Create(spec);
			//case RendererAPIType::OpenGL:  return Ref<OpenGLRenderPass>::Create(spec);
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}