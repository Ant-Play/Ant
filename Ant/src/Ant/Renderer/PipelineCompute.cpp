#include "antpch.h"
#include "PipelineCompute.h"

#include "RendererAPI.h"
#include "Ant/Platform/Vulkan/VulkanComputePipeline.h"

namespace Ant{

	Ref<PipelineCompute> PipelineCompute::Create(Ref<Shader> computeShader)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None: return nullptr;
			case RendererAPIType::Vulkan: return Ref<VulkanComputePipeline>::Create(computeShader);
		}
		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
