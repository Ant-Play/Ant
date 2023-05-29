#pragma once

#include "AssetPackFile.h"

#include "Ant/Core/Buffer.h"
		  
namespace Ant {

	class AssetPackSerializer
	{
	public:
		static void Serialize(const std::filesystem::path& path, AssetPackFile& file, Buffer appBinary, std::atomic<float>& progress);
		static bool DeserializeIndex(const std::filesystem::path& path, AssetPackFile& file);
	private:
		static uint64_t CalculateIndexTableSize(const AssetPackFile& file);
	};

}
