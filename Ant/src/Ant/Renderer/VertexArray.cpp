#include "antpch.h"
#include "Ant/Renderer/VertexArray.h"
#include "Ant/Renderer/Renderer.h"
#include "Ant/Platform/OpenGL/OpenGLVertexArray.h"

namespace Ant {

	Ref<VertexArray> VertexArray::Create()
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:    ANT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPIType::OpenGL:  return Ref<OpenGLVertexArray>::Create();
		}

		ANT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}


}
