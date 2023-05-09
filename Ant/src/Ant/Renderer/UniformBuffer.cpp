#include "antpch.h"
#include "Ant/Renderer/UniformBuffer.h"

#include "Ant/Renderer/Renderer.h"
#include "Ant/Platform/OpenGL/OpenGLUniformBuffer.h"


namespace Ant{

    Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
    {
        switch(Renderer::GetAPI())
        {
            case RendererAPI::API::None:    ANT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLUniformBuffer>(size, binding);
        }

        ANT_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}