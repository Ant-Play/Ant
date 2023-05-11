#include "antpch.h"
#include "Ant/Renderer/Framebuffer.h"
#include "Ant/Renderer/Renderer.h"

#include "Ant/Platform/OpenGL/OpenGLFramebuffer.h"


namespace Ant {


	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		Ref<Framebuffer> result = nullptr;

		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:		return nullptr;
			case RendererAPIType::OpenGL:	result = Ref<OpenGLFramebuffer>::Create(spec);
		}
		FramebufferPool::GetGlobal()->Add(result);
		return result;
	}

	FramebufferPool* FramebufferPool::s_Instance = new FramebufferPool;

	FramebufferPool::FramebufferPool(uint32_t maxFBs /*= 32*/)
	{

	}


	FramebufferPool::~FramebufferPool()
	{

	}


	std::weak_ptr<Ant::Framebuffer> FramebufferPool::AllocateBuffers()
	{
		//return std::weak_ptr<Framebuffer>();
		return {};
	}


	void FramebufferPool::Add(const Ref<Framebuffer> framebuffer)
	{
		m_Pool.push_back(framebuffer);
	}

}
