#pragma once
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace ciallo::vulkan
{
	class Buffer
	{
	private:
		VmaAllocator m_allocator = nullptr;
		VmaAllocation m_allocation = nullptr;
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
		vk::Buffer buffer() const
		{
			return m_buffer;
		}

		void uploadLocal(const void* data, vk::DeviceSize size) const;

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

		bool hostVisible() const;

		bool hostCoherent() const;

		void destroyStagingBuffer();
	};
}
