#pragma once

#include "Ant/Core/Ref.h"

struct GLFWwindow;

namespace Ant{

	class RendererContext : public RefCounted
	{
	public:
		RendererContext() = default;
		virtual ~RendererContext() = default;

		virtual void Init() = 0;

		static Ref<RendererContext> Create();
	};
}