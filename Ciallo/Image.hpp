#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "Buffer.hpp"

namespace ciallo::vulkan
{
	
	/**
	 * \brief 2D image. Copy constructor/assignment only allocate memory, do not copy content.
	 */
	class Image
	{
	private:
		uint32_t m_width = 0u;
		uint32_t m_height = 0u;
		vk::Format m_format = vk::Format::eUndefined;
		vk::ImageLayout m_layout = vk::ImageLayout::eUndefined;
		VmaAllocator m_allocator = nullptr;
		VmaAllocation m_allocation = nullptr;
		vk::Image m_image = VK_NULL_HANDLE;
		std::unique_ptr<Buffer> m_stagingBuffer;
		vk::UniqueImageView m_imageView;
		vk::ImageUsageFlags m_usage;
	public:
		Image(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo, uint32_t width, uint32_t height, vk::ImageUsageFlags usage);
		~Image();

		// TODO: better layout changing, customized image flags
		Image() = default;
		Image(const Image& other);
		Image(Image&& other) noexcept;
		Image& operator=(const Image& other);
		Image& operator=(Image&& other) noexcept;
		operator vk::Image() const {return m_image;}
	public:
		void changeLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout,
		                  vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor);
		vk::ImageMemoryBarrier createLayoutTransitionMemoryBarrier(vk::ImageLayout newLayout,vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor) const;

		void upload(vk::CommandBuffer cb, const void* data, vk::DeviceSize size = 0u);
		vk::DeviceSize size() const;

		void uploadLocal(const void* data, vk::DeviceSize size) const;
		// Upload with provided stagingBuffer
		void uploadStaging(vk::CommandBuffer cb, const void* data, vk::DeviceSize size, vk::Buffer stagingBuffer) const;

	private:
		void genImage(VmaAllocator allocator, VmaAllocationCreateInfo allocInfo, vk::ImageCreateInfo info);
		void genStagingBuffer();
		void genImageView();
		vk::Device device() const;
	public:
		bool hostVisible() const;
		bool hostCoherent() const;

		void setImageLayout(vk::ImageLayout layout)
		{
			m_layout = layout;
		}

		vk::ImageLayout imageLayout() const
		{
			return m_layout;
		}

		vk::ImageView imageView() const
		{
			return *m_imageView;
		}

		uint32_t width() const
		{
			return m_width;
		}

		uint32_t height() const
		{
			return m_height;
		}
		vk::Extent2D extent() const
		{
			return {m_width, m_height};
		}
	};
}
