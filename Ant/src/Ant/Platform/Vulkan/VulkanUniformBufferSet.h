#pragma once

#include "Ant/Renderer/UniformBufferSet.h"

#include <map>

namespace Ant {

	class VulkanUniformBufferSet : public UniformBufferSet
	{
	public:
		VulkanUniformBufferSet(uint32_t frames)
			: m_Frames(frames) {}
		virtual ~VulkanUniformBufferSet() {}

		virtual void Create(uint32_t size, uint32_t binding) override
		{
			for (uint32_t frame = 0; frame < m_Frames; frame++)
			{
				Ref<UniformBuffer> uniformBuffer = UniformBuffer::Create(size, binding);
				Set(uniformBuffer, 0, frame);
			}
		}

		virtual Ref<UniformBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) override
		{
			ANT_CORE_ASSERT(m_UniformBuffers.find(frame) != m_UniformBuffers.end());
			ANT_CORE_ASSERT(m_UniformBuffers.at(frame).find(set) != m_UniformBuffers.at(frame).end());
			ANT_CORE_ASSERT(m_UniformBuffers.at(frame).at(set).find(binding) != m_UniformBuffers.at(frame).at(set).end());

			return m_UniformBuffers.at(frame).at(set).at(binding);
		}

		virtual void Set(Ref<UniformBuffer> uniformBuffer, uint32_t set = 0, uint32_t frame = 0) override
		{
			m_UniformBuffers[frame][set][uniformBuffer->GetBinding()] = uniformBuffer;
		}
	private:
		uint32_t m_Frames;
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, Ref<UniformBuffer>>>> m_UniformBuffers; // frame->set->binding
	};
}