﻿#pragma once

#include "Ant/Renderer/VertexBuffer.h"

#include "Ant/Core/Buffer.h"

#include "VulkanAllocator.h"

namespace Ant{

	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(void* data, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static);
		VulkanVertexBuffer(uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);

		virtual ~VulkanVertexBuffer() override;

		virtual void SetData(void* buffer, uint32_t size, uint32_t offset = 0) override;
		virtual void RT_SetData(void* buffer, uint32_t size, uint32_t offset = 0) override;
		virtual void Bind() const override {}

		virtual unsigned int GetSize() const override { return m_Size; }
		virtual RendererID GetRendererID() const override { return 0; }

		VkBuffer GetVulkanBuffer() const { return m_VulkanBuffer; }
	private:
		uint32_t m_Size = 0;
		Buffer m_LocalData;

		VkBuffer m_VulkanBuffer = nullptr;
		VmaAllocation m_MemoryAllocation;
	};
}
