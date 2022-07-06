#include "pch.hpp"
#include "Image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ciallo::vulkan
{
	Image::Image(VmaAllocator allocator, VmaAllocationCreateInfo allocInfo, uint32_t width, uint32_t height,
	             vk::ImageUsageFlags usage)
	{
		vk::ImageCreateInfo info{};
		info.imageType = vk::ImageType::e2D;
		info.format = vk::Format::eR8G8B8A8Unorm;
		info.extent = vk::Extent3D{width, height, 1u};
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = vk::SampleCountFlagBits::e1;
		info.tiling = vk::ImageTiling::eOptimal;
		info.usage = usage;
		info.sharingMode = vk::SharingMode::eExclusive;
		info.initialLayout = vk::ImageLayout::eUndefined;

		m_allocator = allocator;
		m_width = info.extent.width;
		m_height = info.extent.height;
		m_layout = info.initialLayout;
		m_format = info.format;
		genImage(allocator, allocInfo, info);
		genImageView();
	}

	Image::~Image()
	{
		vmaDestroyImage(m_allocator, m_image, m_allocation);
	}

	/**
	 * \brief Change layout of the image. Barrier do not wait or block command.
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

	vk::ImageMemoryBarrier Image::createLayoutTransitionMemoryBarrier(vk::ImageLayout newLayout,
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
		if (size == 0) size = this->size();
		if (hostVisible())
		{
			uploadLocal(data, size);
		}
		else if (!m_stagingBuffer)
		{
			genStagingBuffer();
			m_stagingBuffer->uploadLocal(data, size);
			uploadStaging(cb, data, size, *m_stagingBuffer);
		}
		else
		{
			m_stagingBuffer->uploadLocal(data, size);
			uploadStaging(cb, data, size, *m_stagingBuffer);
		}
	}

	vk::DeviceSize Image::size() const
	{
		vk::DeviceSize pixelSize = vk::blockSize(m_format);
		return m_width * m_height * pixelSize;
	}

	void Image::genStagingBuffer()
	{
		VmaAllocationCreateInfo info{VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO};
		m_stagingBuffer = std::make_unique<Buffer>(m_allocator, info, size(), vk::BufferUsageFlagBits::eTransferSrc);
	}

	void Image::genImageView()
	{
		vk::ImageViewCreateInfo info{};
		info.setImage(m_image);
		info.setViewType(vk::ImageViewType::e2D);
		info.setFormat(m_format);
		info.setComponents({});
		info.setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
		m_imageView = device().createImageViewUnique(info);
	}

	vk::Device Image::device() const
	{
		VmaAllocatorInfo info;
		vmaGetAllocatorInfo(m_allocator, &info);
		return info.device;
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

	void Image::genImage(VmaAllocator allocator, VmaAllocationCreateInfo allocInfo, vk::ImageCreateInfo info)
	{
		auto i = static_cast<VkImageCreateInfo>(info);
		VkImage image;
		vmaCreateImage(allocator, &i, &allocInfo, &image, &m_allocation, nullptr);
		m_image = image;
	}

	bool Image::hostCoherent() const
	{
		VkMemoryPropertyFlags flags;
		vmaGetAllocationMemoryProperties(m_allocator, m_allocation, &flags);
		return flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
}
