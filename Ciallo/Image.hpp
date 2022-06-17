#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vulkan/vulkan.hpp>

#include <vk_mem_alloc.h>


namespace ciallo::vulkan
{
	class Image
	{
	public:
		Image(VmaAllocator allocator, const std::string& path, 
		      vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
		~Image();
	public:
		void setLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout,
		               vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor);

	private:
		int m_width;
		int m_height;
		vk::Format m_format;
		vk::ImageLayout m_layout;
		VmaAllocator m_allocator;
		VmaAllocation m_allocation;
		VkImage m_vkImage;
		uint32_t m_mipLevels;
		uint32_t m_arrayLayers;
	};
}
