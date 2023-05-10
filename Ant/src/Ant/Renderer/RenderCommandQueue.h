#pragma once
#include "Ant/Renderer/RendererAPI.h"

#include <glm/glm.hpp>
namespace Ant{

	class RenderCommandQueue
	{
		/*public:
			inline static void Init()
			{
				s_RendererAPI->Init();
			}

			inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
			{
				s_RendererAPI->SetViewport(x, y, width, height);
			}

			inline static void SetClearColor(const glm::vec4& color)
			{
				s_RendererAPI->SetClearColor(color);
			}

			inline static void Clear()
			{
				s_RendererAPI->Clear();
			}

			inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
			{
				s_RendererAPI->DrawIndexed(vertexArray, indexCount);
			}

			inline static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
			{
				s_RendererAPI->DrawLines(vertexArray, vertexCount);
			}

			inline static void SetLineWidth(float width)
			{
				s_RendererAPI->SetLineWidth(width);
			}

		private:
			static Scope<RendererAPI> s_RendererAPI;*/
	public:
		typedef void(*RenderCommandFn)(void*);

		RenderCommandQueue();
		~RenderCommandQueue();

		void* Allocate(RenderCommandFn func, uint32_t size);

		void Execute();
	private:
		uint8_t* m_CommandBuffer;
		uint8_t* m_CommandBufferPtr;
		uint32_t m_CommandCount = 0;
	};
}
