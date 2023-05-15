#include "antpch.h"
#include "Framebuffer.h"

#include "Ant/Platform/OpenGL/OpenGLFramebuffer.h"
#include "Ant/Platform/Vulkan/VulkanFramebuffer.h"

#include "RendererAPI.h"

namespace Ant {


	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		Ref<Framebuffer> result = nullptr;

		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:		return nullptr;
			case RendererAPIType::OpenGL:	result = Ref<OpenGLFramebuffer>::Create(spec); break;
			case RendererAPIType::Vulkan:	result = Ref<VulkanFramebuffer>::Create(spec); break;
		}
		FramebufferPool::GetGlobal()->Add(result);
		return result;
	}

	FramebufferPool* FramebufferPool::s_Instance = new FramebufferPool;

	FramebufferPool::FramebufferPool(uint32_t maxFBs /* = 32 */)
	{

	}

	FramebufferPool::~FramebufferPool()
	{

	}

	std::weak_ptr<Framebuffer> FramebufferPool::AllocateBuffer()
	{
		// m_Pool.push_back();
		return std::weak_ptr<Framebuffer>();
	}

	void FramebufferPool::Add(const Ref<Framebuffer>& framebuffer)
	{
		m_Pool.push_back(framebuffer);
	}

}
