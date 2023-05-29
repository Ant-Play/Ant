#pragma once

#include "Ant/Project/Project.h"

namespace Ant{

	class ScriptBuilder
	{
	public:
		static void BuildCSProject(const std::filesystem::path& filepath);
		static void BuildScriptAssembly(Ref<Project> project);
	};
}