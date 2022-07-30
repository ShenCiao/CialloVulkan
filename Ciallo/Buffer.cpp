#include "pch.hpp"

#include "Buffer.hpp"

namespace ciallo::vulkan
{
	vk::MemoryPropertyFlags AllocationBase::memoryProperty() const
	{
		VkMemoryPropertyFlags flags;
		vmaGetAllocationMemoryProperties(m_allocator, m_allocation, &flags);
		return static_cast<vk::MemoryPropertyFlags>(flags);
	}

	bool AllocationBase::hostVisible() const
	{
		return static_cast<bool>(memoryProperty() & vk::MemoryPropertyFlagBits::eHostVisible);
	}

	bool AllocationBase::hostCoherent() const
	{
		return static_cast<bool>(memoryProperty() & vk::MemoryPropertyFlagBits::eHostCoherent);
	}

	uint32_t AllocationBase::memoryTypeIndex() const
	{
		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(m_allocator, m_allocation, &allocInfo);
		return allocInfo.memoryType;
	}

	void AllocationBase::uploadLocal(const void* data, vk::DeviceSize size) const
	{
		void* mappedData;
		vmaMapMemory(m_allocator, m_allocation, &mappedData);
		memcpy(mappedData, data, size);
		if (!hostCoherent())
		{
			vmaFlushAllocation(m_allocator, m_allocation, 0, VK_WHOLE_SIZE);
		}
		vmaUnmapMemory(m_allocator, m_allocation);
	}

	vk::Device AllocationBase::device() const
	{
		VmaAllocatorInfo info;
		vmaGetAllocatorInfo(m_allocator, &info);
		return info.device;
	}

	AllocationBase::AllocationBase(VmaAllocator allocator): m_allocator(allocator)
	{
	}

	AllocationBase::AllocationBase(AllocationBase&& other) noexcept
	{
		*this = std::move(other);
	}

	AllocationBase& AllocationBase::operator=(AllocationBase&& other) noexcept
	{
		using std::swap;
		swap(m_allocator, other.m_allocator);
		swap(m_allocation, other.m_allocation);
		return *this;
	}

	Buffer::Buffer(VmaAllocator allocator, VmaAllocationCreateInfo allocInfo, vk::DeviceSize size,
	               vk::BufferUsageFlags usage): AllocationBase(allocator), m_size(size), m_usage(usage)
	{
		vk::BufferCreateInfo info{{}, size, usage};

		auto i = static_cast<VkBufferCreateInfo>(info);
		VkBuffer buffer;
		vmaCreateBuffer(allocator, &i, &allocInfo, &buffer, &m_allocation, nullptr);

		m_buffer = buffer;
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

	void Buffer::destroyStagingBuffer()
	{
		m_stagingBuffer.reset();
	}

	Buffer::Buffer(const Buffer& other)
	{
		*this = other;
	}

	Buffer::Buffer(Buffer&& other) noexcept
	{
		*this = std::move(other);
	}

	Buffer& Buffer::operator=(const Buffer& other)
	{
		if (!other.m_allocator) // other is default constructed, default construct this
		{
			*this = Buffer();
			return *this;
		}

		uint32_t index = memoryTypeIndex();
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.memoryTypeBits = 1u << index;

		// Call move assignment operator.
		*this = Buffer(other.m_allocator, allocCreateInfo, m_size, m_usage);
		return *this;
	}

	Buffer& Buffer::operator=(Buffer&& other) noexcept
	{
		AllocationBase::operator=(std::move(other));
		using std::swap;
		swap(m_buffer, other.m_buffer);
		swap(m_size, other.m_size);
		swap(m_stagingBuffer, other.m_stagingBuffer);
		return *this;
	}

	Buffer::~Buffer()
	{
		if (m_allocator && m_buffer && m_allocation)
		{
			vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
		}
	}

	vk::Buffer Buffer::buffer() const
	{
		return m_buffer;
	}
}
