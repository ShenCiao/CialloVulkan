#pragma once
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace ciallo::vulkan
{
	class Buffer
	{
	private:
		VmaAllocator m_allocator;
		VmaAllocation m_allocation;
		vk::Buffer m_buffer;
		vk::DeviceSize m_size;
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

		Buffer() = delete;
		Buffer(const Buffer& other) = delete;
		Buffer(Buffer&& other) = default;
		Buffer& operator=(const Buffer& other) = delete;
		Buffer& operator=(Buffer&& other) = default;
		~Buffer();

	public:
		vk::Buffer buffer() const
		{
			return m_buffer;
		}

		void uploadLocal(const void* data, vk::DeviceSize size) const;

		// Upload with provided stagingBuffer
		void uploadStaging(vk::CommandBuffer cb, const void* data, vk::DeviceSize size, vk::Buffer stagingBuffer) const;

		void genStagingBuffer()
		{
			VmaAllocationCreateInfo info{VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO};
			m_stagingBuffer = std::make_unique<Buffer>(m_allocator, info, m_size, vk::BufferUsageFlagBits::eTransferSrc);
		}

		/**
		 * \brief Upload data to buffer
		 * \param cb Command buffer for recording copy command
		 * \param data Pointer to the data
		 * \param size Size of the data
		 */
		void upload(vk::CommandBuffer cb, const void* data, vk::DeviceSize size);

		template <typename T>
		void upload(vk::CommandBuffer cb, std::vector<T>& data);


		template <typename T>
		void upload(vk::CommandBuffer cb, T& data);

		bool hostVisible() const;

		bool hostCoherent() const;

		void destroyStagingBuffer();
	};
}
