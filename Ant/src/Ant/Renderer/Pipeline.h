#pragma once

#include "RenderPass.h"
#include "Ant/Core/Ref.h"

#include "VertexBuffer.h"
#include "Shader.h"
#include "RenderPass.h"


namespace Ant{

	class RenderPass;

	struct PipelineSpecification
	{
		Ref<Shader> Shader;
		VertexBufferLayout Layout;

		Ref<RenderPass> RenderPass;

		std::string DebugName;
	};


	class Pipeline : public RefCounted
	{
	public:

		virtual ~Pipeline() = default;

		virtual PipelineSpecification& GetSpecification() = 0;
		virtual const PipelineSpecification& GetSpecification() const = 0;

		virtual void Invalidate() = 0;

		// TEMP: remove this when render command buffers are a thing
		virtual void Bind() = 0;

		static Ref<Pipeline> Create(const PipelineSpecification& spec);
	};

}