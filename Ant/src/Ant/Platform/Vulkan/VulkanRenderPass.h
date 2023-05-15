#pragma once

#include "Ant/Renderer/RenderPass.h"

namespace Ant{

	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& spec);
		virtual ~VulkanRenderPass();

		virtual RenderPassSpecification& GetSpecification() override { return m_Specification; }
		virtual const RenderPassSpecification& GetSpecification() const override { return m_Specification; }
	private:
		RenderPassSpecification m_Specification;
	};
}
