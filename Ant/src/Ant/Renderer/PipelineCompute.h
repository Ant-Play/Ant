#pragma once

#include "Ant/Core/Base.h"
#include "Shader.h"
#include "RenderCommandBuffer.h"

namespace Ant{

	class PipelineCompute : public RefCounted
	{
	public:
		virtual void Begin(Ref<RenderCommandBuffer> renderCommandBuffer = nullptr) = 0;
		virtual void RT_Begin(Ref<RenderCommandBuffer> renderCommandBuffer = nullptr) = 0;
		virtual void End() = 0;

		virtual Ref<Shader> GetShader() = 0;

		static Ref<PipelineCompute> Create(Ref<Shader> computeShader);
	};
}
