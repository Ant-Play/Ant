#include "antpch.h"
#include "Ant/Renderer/Buffer.h"
#include "Ant/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Ant {
	
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				ANT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			}
			case RendererAPI::API::OpenGL:
			{
				return CreateRef<OpenGLVertexBuffer>(size);
			}
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{

		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: 
			{
				ANT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			}
			case RendererAPI::API::OpenGL:
			{
				return CreateRef<OpenGLVertexBuffer>(vertices, size);
			}
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}


	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{

		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				ANT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			}
			case RendererAPI::API::OpenGL:
			{
				return CreateRef<OpenGLIndexBuffer>(indices, size);
			}
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
