﻿#pragma once

#include "Ant/Asset/Asset.h"
#include "Ant/Asset/AssetMetadata.h"

#include <optional>

namespace Ant::AudioFileUtils {

	struct  AudioFileInfo
	{
		double Duration;
		uint32_t SamplingRate;
		uint16_t BitDepth;
		uint16_t NumChannels;
		uint64_t FileSize;
	};

	std::optional<AudioFileInfo> GetFileInfo(const AssetMetadata& metadata);
	std::optional<AudioFileInfo> GetFileInfo(const std::filesystem::path& filepath);

	bool IsValidAudioFile(const std::filesystem::path& filepath);

	std::string ChannelsToLayoutString(uint16_t numChannels);
}
