#include "antpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Ant{


	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		ANT_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init()
	{
		//这里将一个glfwWindow设置为当前上下文，一个thread同时只能拥有一个上下文，
		//这省去了一些函数每次都指定window的麻烦，像glfwSwapInterval()这样的函数只操作当前Context
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ANT_CORE_ASSERT(status, "Failed to initialize Glad!");

		ANT_CORE_INFO("OpenGL Info:");
		ANT_CORE_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		ANT_CORE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
		ANT_CORE_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));

		ANT_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "Ant requires at least OpenGL version 4.5!");
	}


	void OpenGLContext::SwapBuffers()
	{
		//刷新下一帧(严格来说是把Framebuffer后台帧换到前台，把Framebuffer当前帧换到后台，
		//所以是Swap)
		glfwSwapBuffers(m_WindowHandle);
	}

}
