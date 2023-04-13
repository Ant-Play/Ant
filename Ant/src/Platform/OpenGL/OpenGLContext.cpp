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
		//���ｫһ��glfwWindow����Ϊ��ǰ�����ģ�һ��threadͬʱֻ��ӵ��һ�������ģ�
		//��ʡȥ��һЩ����ÿ�ζ�ָ��window���鷳����glfwSwapInterval()�����ĺ���ֻ������ǰContext
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
		//ˢ����һ֡(�ϸ���˵�ǰ�Framebuffer��̨֡����ǰ̨����Framebuffer��ǰ֡������̨��
		//������Swap)
		glfwSwapBuffers(m_WindowHandle);
	}

}
