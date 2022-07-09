#include "pch.hpp"

#include "Buffer.hpp"

namespace ciallo::vulkan
{
	Buffer::Buffer(VmaAllocator allocator, VmaAllocationCreateInfo allocInfo, vk::DeviceSize size,
	               vk::BufferUsageFlags usage): m_allocator(allocator), m_size(size)
	{
		vk::BufferCreateInfo info{{}, size, usage};
		auto i = static_cast<VkBufferCreateInfo>(info);

		VkBuffer buffer;
		vmaCreateBuffer(allocator, &i, &allocInfo, &buffer, &m_allocation, nullptr);
		m_buffer = buffer;
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

	void Buffer::genStagingBuffer()
	{
		VmaAllocationCreateInfo info{VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO};
		m_stagingBuffer = std::make_unique<Buffer>(m_allocator, info, m_size, vk::BufferUsageFlagBits::eTransferSrc);
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

	Buffer::Buffer(Buffer&& other) noexcept
	{
		*this = std::move(other);
	}

	Buffer& Buffer::operator=(Buffer&& other) noexcept
	{
		std::swap(m_allocator, other.m_allocator);
		std::swap(m_buffer, other.m_buffer);
		std::swap(m_allocation, other.m_allocation);
		std::swap(m_size, other.m_size);
		std::swap(m_stagingBuffer, other.m_stagingBuffer);
		return *this;
	}

	Buffer::~Buffer()
	{
		if (m_allocator)
		{
			vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
		}
	}
}
