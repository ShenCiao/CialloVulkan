#pragma once
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace ciallo::vulkan
{
	// Base class for objects need to allocate memory with vulkan memory allocator. Include Buffer, Image and Mapper.
	class AllocationBase
	{
	protected:
		VmaAllocator m_allocator = nullptr;
		VmaAllocation m_allocation = nullptr;

	public:
		AllocationBase() = default;
		explicit AllocationBase(VmaAllocator allocator);
		AllocationBase(const AllocationBase& other) = delete;
		AllocationBase(AllocationBase&& other) noexcept;
		AllocationBase& operator=(const AllocationBase& other) = delete;
		AllocationBase& operator=(AllocationBase&& other) noexcept;
		~AllocationBase() = default;

		vk::MemoryPropertyFlags memoryProperty() const;
		bool hostVisible() const;
		bool hostCoherent() const;
		uint32_t memoryTypeIndex() const;
		vk::Device device() const;

		void uploadLocal(const void* data, vk::DeviceSize offset, vk::DeviceSize size) const;
	};

	class Buffer : public AllocationBase
	{
		vk::Buffer m_buffer = VK_NULL_HANDLE;
		vk::DeviceSize m_size{};
		vk::BufferUsageFlags m_usage;
		std::unique_ptr<Buffer> m_stagingBuffer;
	public:
		/**
		 * \brief Create buffer with vulkan memory allocator
		 * \param allocator allocator
		 * \param allocInfo VmaAllocationCreateInfo. Set VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT for being host writable;
		 * \param size Size of buffer
		 * \param usage Usage of buffer
		 */
		Buffer(VmaAllocator allocator, VmaAllocationCreateInfo allocInfo, vk::DeviceSize size,
		       vk::BufferUsageFlags usage);

		operator vk::Buffer() const { return m_buffer; }

		Buffer() = default;
		Buffer(const Buffer& other);
		Buffer(Buffer&& other) noexcept;
		Buffer& operator=(const Buffer& other);
		Buffer& operator=(Buffer&& other) noexcept;
		~Buffer();

	public:
		vk::Buffer buffer() const;
		// Upload with provided stagingBuffer
		void uploadStaging(vk::CommandBuffer cb, const void* data, vk::DeviceSize size, vk::Buffer stagingBuffer) const;
		void genStagingBuffer();
		/**
		 * \brief Upload data to buffer
		 * \param cb Command buffer for recording copy command
		 * \param data Pointer to the data
		 * \param size Size of the data
		 */
		void upload(vk::CommandBuffer cb, const void* data, vk::DeviceSize size);
		template <typename T>
		void upload(vk::CommandBuffer cb, std::vector<T>& data);
		void uploadLocal(const void* data, vk::DeviceSize size) const;

		vk::DeviceSize size() const;
		void destroyStagingBuffer();
	};
}
