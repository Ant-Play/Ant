#pragma once

#include <string>

namespace Ant{

	class FileDialogs
	{
	public:
		// These reture empty strings if cancelled
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};
}
