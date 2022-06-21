#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vulkan/vulkan.hpp>

#include <vk_mem_alloc.h>

#include "Buffer.hpp"


namespace ciallo::vulkan
{
	class Image
	{
	private:
		uint32_t m_width;
		uint32_t m_height;
		vk::Format m_format;
		vk::ImageLayout m_layout;
		VmaAllocator m_allocator;
		VmaAllocation m_allocation;
		VkImage m_image;
		std::unique_ptr<Buffer> m_stagingBuffer;
	public:
		Image(VmaAllocator allocator, const std::string& path,
		      vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
		      VmaAllocationCreateFlags flags = {});
		Image(VmaAllocator allocator, vk::ImageCreateInfo info, VmaAllocationCreateInfo allocInfo);
		~Image();
	public:
		void changeLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout,
		               vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor);
		vk::ImageMemoryBarrier generateLayoutChangingMemoryBarrier(vk::ImageLayout newLayout,
		                                                           vk::ImageAspectFlags aspectMask =
		                                                                  vk::ImageAspectFlagBits::eColor) const;

		void upload(vk::CommandBuffer cb, const void* data, vk::DeviceSize size);
		vk::DeviceSize size() const;

		void uploadLocal(const void* data, vk::DeviceSize size) const;
		// Upload with provided stagingBuffer
		void uploadStaging(vk::CommandBuffer cb, const void* data, vk::DeviceSize size, vk::Buffer stagingBuffer) const;


		void createStagingBuffer();
		bool hostVisible() const;
		bool hostCoherent() const;
		void setLayout(vk::ImageLayout layout)
		{
			m_layout = layout;
		}
	};
}
