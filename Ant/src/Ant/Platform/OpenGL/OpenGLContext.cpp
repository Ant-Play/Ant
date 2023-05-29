#include "antpch.h"
//#include "OpenGLContext.h"
//
//#include <glad/glad.h>
//#include <glfw/glfw3.h>
//
//#include "Ant/Core/Log.h"
//
//namespace Ant{
//
//	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
//		: m_WindowHandle(windowHandle)
//	{
//	}
//
//	OpenGLContext::~OpenGLContext()
//	{
//	}
//
//	void OpenGLContext::Create()
//	{
//		ANT_CORE_INFO("OpenGLContext::Create");
//
//		glfwMakeContextCurrent(m_WindowHandle);
//		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
//		ANT_CORE_ASSERT(status, "Failed to initialize Glad!");
//
//		ANT_CORE_INFO("OpenGL Info:");
//		ANT_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
//		ANT_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
//		ANT_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));
//
//#ifdef ANT_ENABLE_ASSERTS
//		int versionMajor;
//		int versionMinor;
//		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
//		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
//
//		ANT_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Ant requires at least OpenGL version 4.5!");
//#endif
//	}
//
//	void OpenGLContext::SwapBuffers()
//	{
//		glfwSwapBuffers(m_WindowHandle);
//	}
//}
