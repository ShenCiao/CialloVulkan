#include "pch.hpp"

#include "Buffer.hpp"

namespace ciallo::vulkan
{
	Buffer::Buffer(VmaAllocator allocator, vk::DeviceSize size, vk::BufferUsageFlags usage,
		VmaAllocationCreateFlags flags)
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
		allocInfo.flags = flags;

		vmaCreateBuffer(allocator, &bufferCreateInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
	}

	void Buffer::uploadLocal(const void* data, vk::DeviceSize size) const
	{
		if (!hostVisible())
		{
			throw std::runtime_error("Buffer is not visible to host");
		}
		void* mappedData;
		vmaMapMemory(m_allocator, m_allocation, &mappedData);
		memcpy(mappedData, data, size);
		if (!hostCoherent())
		{
			vmaFlushAllocation(m_allocator, m_allocation, 0, VK_WHOLE_SIZE);
		}
		vmaUnmapMemory(m_allocator, m_allocation);
	}

	void Buffer::uploadStaging(vk::CommandBuffer cb, const void* data, vk::DeviceSize size,
		vk::Buffer stagingBuffer) const
	{
		m_stagingBuffer->uploadLocal(data, size);
		vk::BufferCopy bc{0, 0, size};
		cb.copyBuffer(stagingBuffer, m_buffer, bc);
	}


	void Buffer::upload(vk::CommandBuffer cb, const void* data, vk::DeviceSize size)
	{
		if (size == 0) return;
		if (hostVisible())
		{
			uploadLocal(data, size);
		}
		else if (m_stagingBuffer)
		{
			uploadStaging(cb, data, size, *m_stagingBuffer);
		}
		else
		{
			genStagingBuffer();
			uploadStaging(cb, data, size, *m_stagingBuffer);
		}
	}

	template <typename T>
	void Buffer::upload(vk::CommandBuffer cb, std::vector<T>& data)
	{
		upload(cb, data.data(), data.size() * sizeof(T));
	}

	template <typename T>
	void Buffer::upload(vk::CommandBuffer cb, T& data)
	{
		upload(cb, data.data(), data.size() * sizeof(T));
	}

	bool Buffer::hostVisible() const
	{
		VkMemoryPropertyFlags flags;
		vmaGetAllocationMemoryProperties(m_allocator, m_allocation, &flags);
		return flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	}

	bool Buffer::hostCoherent() const
	{
		VkMemoryPropertyFlags flags;
		vmaGetAllocationMemoryProperties(m_allocator, m_allocation, &flags);
		return flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}

	void Buffer::destroyStagingBuffer()
	{
		m_stagingBuffer.reset();
	}

	Buffer::~Buffer()
	{
		vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
	}
}
