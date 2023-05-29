#pragma once

#include "Asset.h"
#include "AssetSerializer.h"

#include "Ant/Serialization/AssetPack.h"
#include "Ant/Serialization/AssetPackFile.h"
#include "Ant/Serialization/FileStream.h"

namespace Ant {

	class MeshRuntimeSerializer
	{
	public:
		bool SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo);
		Ref<Asset> DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo);
	};

}
