#pragma once

#include "Ant/Core/Buffer.h"

#include <filesystem>

namespace Ant {
	class FileSystem
	{
	public:
		// TODO: move to FileSystem class
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
	};
}
