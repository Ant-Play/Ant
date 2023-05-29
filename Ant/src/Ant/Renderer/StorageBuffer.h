#pragma once

#include "Ant/Core/Ref.h"

namespace Ant {

	class StorageBuffer : public RefCounted
	{
	public:
		virtual ~StorageBuffer() = default;
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
		virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Resize(uint32_t newSize) = 0;

		virtual uint32_t GetBinding() const = 0;

		static Ref<StorageBuffer> Create(uint32_t size, uint32_t binding);
	};

}
