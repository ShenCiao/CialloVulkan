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
		std::unique_ptr<Buffer> m_stagingBuffer;
	public:
		/**
		 * \brief Create buffer with vulkan memory allocator, host INVISIBLE by default.
		 * \param allocator allocator
		 * \param size Size of buffer
		 * \param usage Usage in VkBufferCreateInfo
		 * \param flags Flags in VmaAllocationCreateInfo. Set VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT for being host writable;
		 */
		Buffer(VmaAllocator allocator, vk::DeviceSize size, vk::BufferUsageFlags usage,
		       VmaAllocationCreateFlags flags = {});

		operator vk::Buffer() const { return m_buffer; }

		Buffer(const Buffer& other) = delete;
		Buffer(Buffer&& other) = default;
		Buffer& operator=(const Buffer& other) = delete;
		Buffer& operator=(Buffer&& other) = default;
		~Buffer();

		void uploadLocal(const void* data, vk::DeviceSize size) const;

		// Upload with provided stagingBuffer
		void uploadStaging(vk::CommandBuffer cb, const void* data, vk::DeviceSize size, vk::Buffer stagingBuffer) const;

		void genStagingBuffer()
		{
			m_stagingBuffer = std::make_unique<Buffer>(m_allocator, m_size, vk::BufferUsageFlagBits::eTransferSrc,
		                                           VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		}

		/**
		 * \brief Upload data to buffer
		 * \param cb Command buffer for recording copy command
		 * \param data Pointer to data
		 * \param size Size of data
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
