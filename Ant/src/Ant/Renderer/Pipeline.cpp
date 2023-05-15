#include "antpch.h"
#include "Pipeline.h"

#include "Renderer.h"

#include "Ant/Platform/OpenGL/OpenGLPipeline.h"
#include "Ant/Platform/Vulkan/VulkanPipeline.h"

#include "RendererAPI.h"

namespace Ant{

	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:    return nullptr;
			case RendererAPIType::OpenGL:  return Ref<OpenGLPipeline>::Create(spec);
			case RendererAPIType::Vulkan:  return Ref<VulkanPipeline>::Create(spec);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}