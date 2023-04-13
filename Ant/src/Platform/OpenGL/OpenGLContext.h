#pragma once

#include "Ant/Renderer/GraphicContext.h"

struct GLFWwindow;

namespace Ant {

	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;

	};
}
