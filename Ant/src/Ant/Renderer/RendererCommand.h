#pragma once
#include "Ant/Core.h"
#include "Ant/Renderer/RendererAPI.h"

namespace Ant{

	class RendererCommand
	{
	public:
		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& VertexArray)
		{
			s_RendererAPI->DrawIndexed(VertexArray);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};
}
