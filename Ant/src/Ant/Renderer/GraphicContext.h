#pragma once

namespace Ant
{
	//上下文抽象
	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
	};
}
