﻿#pragma once

#include "Ant/Core/Base.h"
#include "Ant/Core/Math/AABB.h"

#include <map>

namespace Ant {

	struct MeshSourceFile
	{
		enum class MeshFlags : uint32_t
		{
			HasMaterials = BIT(0),
			HasAnimation = BIT(1),
			HasSkeleton = BIT(2)
		};

		struct Metadata
		{
			uint32_t Flags;
			AABB BoundingBox;

			uint64_t NodeArrayOffset;
			uint64_t NodeArraySize;

			uint64_t SubmeshArrayOffset;
			uint64_t SubmeshArraySize;

			uint64_t MaterialArrayOffset;
			uint64_t MaterialArraySize;

			uint64_t VertexBufferOffset;
			uint64_t VertexBufferSize;

			uint64_t IndexBufferOffset;
			uint64_t IndexBufferSize;

			uint64_t AnimationDataOffset;
			uint64_t AnimationDataSize;
		};

		struct FileHeader
		{
			const char HEADER[5] = { 'A','N','T','M','S'};
			uint32_t Version = 1;
			// other metadata?
		};

		FileHeader Header;
		Metadata Data;
	};

}