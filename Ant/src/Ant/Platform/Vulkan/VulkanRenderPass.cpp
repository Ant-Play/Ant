#include "antpch.h"
#include "VulkanRenderPass.h"

namespace Ant{

	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec)
		: m_Specification(spec)
	{
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
	}
}
