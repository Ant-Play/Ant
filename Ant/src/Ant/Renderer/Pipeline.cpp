#include "antpch.h"
#include "Ant/Renderer/Pipeline.h"

#include "Ant/Renderer/Renderer.h"

#include "Ant/Platform/OpenGL/OpenGLPipeline.h"


namespace Ant{

	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLPipeline>::Create(spec);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}