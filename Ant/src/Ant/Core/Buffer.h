#pragma once

#include "Ant/Core/Base.h"

#include <stdint.h>
#include <cstring>

namespace Ant {
	// Non-owning raw buffer class
	struct Buffer
	{
		byte* Data = nullptr;
		uint64_t Size = 0;

		Buffer()
			: Data(nullptr), Size(0)
		{}

		Buffer(uint64_t size)
		{
			Allocate(size);
		}

		Buffer(byte* data, uint64_t size)
			: Data(data), Size(size)
		{}

		Buffer(const Buffer&) = default;

		static Buffer Copy(Buffer other)
		{
			Buffer result(other.Size);
			memcpy(result.Data, other.Data, other.Size);
			return result;
		}

		static Buffer Copy(void* data, uint64_t size)
		{
			Buffer result;
			result.Allocate(size);
			memcpy(result.Data, data, size);
			return result;
		}

		void Allocate(uint64_t size)
		{
			Release();

			Data = new uint8_t[size];
			Size = size;
		}

		void Release()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0;
		}

		void ZeroInitialize()
		{
			if (Data)
				memset(Data, 0, Size);
		}

		void Write(void* data, uint64_t size, uint64_t offset = 0)
		{
			ANT_CORE_ASSERT(offset + size <= Size, "Buffer overflow");
			memcpy(Data + offset, data, size);
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		operator bool() const
		{
			return (bool)Data;
		}

		byte& operator[](int index)
		{
			return Data[index];
		}

		byte& operator[](int index) const
		{
			return Data[index];
		}
	};

}
