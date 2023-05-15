#pragma once

#include "Ant/Core/Base.h"

#include "Framebuffer.h"

namespace Ant{

	class Framebuffer;

	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
		std::string DebugName;
	};

	class RenderPass : public RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification() = 0;
		virtual const RenderPassSpecification& GetSpecification() const = 0;

		static Ant::Ref<Ant::RenderPass> Create(const RenderPassSpecification& spec);
	};
}