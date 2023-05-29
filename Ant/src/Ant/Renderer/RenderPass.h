#pragma once

#include "Ant/Core/Base.h"

#include "Framebuffer.h"

namespace Ant{

	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
		std::string DebugName;
		glm::vec4 MarkerColor;
	};

	class RenderPass : public RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification() = 0;
		virtual const RenderPassSpecification& GetSpecification() const = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);
	};
}