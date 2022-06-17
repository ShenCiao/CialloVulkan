#pragma once
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace ciallo::vulkan
{
	class Buffer
	{
	private:
		vk::DeviceSize m_size;
		VkBuffer m_buffer;
		VmaAllocation m_allocation;
		VmaAllocator m_allocator;
	public:
		Buffer(VmaAllocator allocator, vk::DeviceSize size, vk::BufferUsageFlags usage)
		{
			m_allocator = allocator;
			VkBufferCreateInfo bufferCreateInfo;
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.pNext = nullptr;
			bufferCreateInfo.flags = {};
			bufferCreateInfo.size = m_size = size;
			bufferCreateInfo.usage = static_cast<VkBufferUsageFlags>(usage);
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

			vmaCreateBuffer(allocator, &bufferCreateInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
		}

		~Buffer()
		{
			vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
		}
	};
}
