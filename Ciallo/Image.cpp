#include "pch.hpp"
#include "Image.hpp"

namespace ciallo::vulkan
{
	Image::Image(VmaAllocator allocator, const std::string& path, vk::ImageUsageFlags usage,
	             VmaAllocationCreateFlags flags)
	{
		int width, height, channels;
		uint8_t* data = nullptr;

		if (stbi_is_hdr(path.c_str()))
		{
			data = reinterpret_cast<uint8_t*>(stbi_loadf(path.c_str(), &width, &height, &channels, 4));
			m_format = vk::Format::eR32G32B32A32Sfloat;
		}
		else
		{
			data = stbi_load(path.c_str(), &width, &height, &channels, 4);
			m_format = vk::Format::eR8G8B8A8Unorm;
		}

		m_allocator = allocator;
		m_width = width;
		m_height = height;
		m_layout = vk::ImageLayout::eUndefined;

		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = static_cast<VkFormat>(m_format);
		info.extent.width = m_width;
		info.extent.height = m_height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = static_cast<VkImageUsageFlags>(usage);
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = flags;

		vmaCreateImage(allocator, &info, &allocInfo, &m_image, &m_allocation, nullptr);


		free(data);
	}

	Image::Image(VmaAllocator allocator, vk::ImageCreateInfo info, VmaAllocationCreateInfo allocInfo)
	{
		m_allocator = allocator;
		m_width = info.extent.width;
		m_height = info.extent.height;
		m_layout = info.initialLayout;
		m_format = info.format;

		auto i = static_cast<VkImageCreateInfo>(info);
		vmaCreateImage(allocator, &i, &allocInfo, &m_image, &m_allocation, nullptr);
	}

	Image::~Image()
	{
		vmaDestroyImage(m_allocator, m_image, m_allocation);
	}

	/**
	 * \brief Change layout of the image.
	 */
	void Image::changeLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask)
	{
		if (newLayout == m_layout) return;
		vk::ImageLayout oldLayout = m_layout;
		m_layout = newLayout;

		vk::ImageMemoryBarrier imageMemoryBarriers = {};
		imageMemoryBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarriers.oldLayout = oldLayout;
		imageMemoryBarriers.newLayout = newLayout;
		imageMemoryBarriers.image = m_image;
		imageMemoryBarriers.subresourceRange = {aspectMask, 0, 1, 0, 1};

		vk::PipelineStageFlags srcStageMask{vk::PipelineStageFlagBits::eTopOfPipe}; // wait for nothing
		vk::PipelineStageFlags dstStageMask{vk::PipelineStageFlagBits::eBottomOfPipe}; // block nothing
		vk::DependencyFlags dependencyFlags{};
		vk::AccessFlags srcMask{};
		vk::AccessFlags dstMask{};

		imageMemoryBarriers.srcAccessMask = srcMask;
		imageMemoryBarriers.dstAccessMask = dstMask;
		auto memoryBarriers = nullptr;
		auto bufferMemoryBarriers = nullptr;
		cb.pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags, memoryBarriers, bufferMemoryBarriers,
		                   imageMemoryBarriers);
	}

	// m_layout do not changed in this function
	vk::ImageMemoryBarrier Image::generateLayoutChangingMemoryBarrier(vk::ImageLayout newLayout,
	                                                                  vk::ImageAspectFlags aspectMask) const
	{
		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = m_layout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.image = m_image;
		imageMemoryBarrier.subresourceRange = {aspectMask, 0, 1, 0, 1};

		return imageMemoryBarrier;
	}

	// Warning: mipmap and array layers unsupported
	void Image::upload(vk::CommandBuffer cb, const void* data, vk::DeviceSize size)
	{
		if (size == 0) return;
		if (hostVisible())
		{
			uploadLocal(data, size);
		}
		else if (!m_stagingBuffer)
		{
			createStagingBuffer();
			uploadStaging(cb, data, size, *m_stagingBuffer);
		}
		else
		{
			uploadStaging(cb, data, size, *m_stagingBuffer);
		}
	}

	vk::DeviceSize Image::size() const
	{
		vk::DeviceSize pixelSize = vk::blockSize(m_format) * sizeof(uint8_t);
		return m_width * m_height * pixelSize;
	}

	void Image::createStagingBuffer()
	{
		m_stagingBuffer = std::make_unique<Buffer>(m_allocator, size(), vk::BufferUsageFlagBits::eTransferSrc,
		                                           VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
	}

	bool Image::hostVisible() const
	{
		VkMemoryPropertyFlags flags;
		vmaGetAllocationMemoryProperties(m_allocator, m_allocation, &flags);
		return flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	}

	void Image::uploadLocal(const void* data, vk::DeviceSize size) const
	{
		if (!hostVisible())
		{
			throw std::runtime_error("Image is not visible to host");
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

	// Only color image for now.
	void Image::uploadStaging(vk::CommandBuffer cb, const void* data, vk::DeviceSize size,
	                          vk::Buffer stagingBuffer) const
	{
		// Refer to vulkan spec struct VkImageSubresourceRange for aspectMask information
		vk::BufferImageCopy copy{};
		copy.setBufferOffset(0);
		copy.setImageExtent({m_width, m_height, 1u});
		copy.setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});
		cb.copyBufferToImage(stagingBuffer, m_image, m_layout, copy);
	}

	bool Image::hostCoherent() const
	{
		VkMemoryPropertyFlags flags;
		vmaGetAllocationMemoryProperties(m_allocator, m_allocation, &flags);
		return flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
}
