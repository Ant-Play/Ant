#pragma once

#include "Ant/Renderer/RenderPass.h"

namespace Ant{

	class OpenGLRenderPass : public RenderPass
	{
	public:
		OpenGLRenderPass(const RenderPassSpecification& spec);
		virtual ~OpenGLRenderPass();

		virtual RenderPassSpecification& GetSpecification() override { return m_Specification; }
		virtual const RenderPassSpecification& GetSpecification() const override { return m_Specification; }
	private:
		RenderPassSpecification m_Specification;
	};
}