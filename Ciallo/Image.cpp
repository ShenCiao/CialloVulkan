#include "pch.hpp"
#include "Image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ciallo::vulkan
{
	Image::Image(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo, vk::ImageCreateInfo info):
		AllocationBase(allocator), m_format(info.format), m_width(info.extent.width), m_height(info.extent.height),
		m_usage(info.usage), m_layout(info.initialLayout)
	{
		genImage(allocator, allocCreateInfo, info);
		genImageView();
	}

	Image::Image(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo, uint32_t width, uint32_t height,
	             vk::ImageUsageFlags usage): AllocationBase(allocator), m_width(width), m_height(height), m_usage(usage)
	{
		vk::ImageCreateInfo info{};
		info.imageType = vk::ImageType::e2D;
		info.format = vk::Format::eR8G8B8A8Unorm;
		info.extent = vk::Extent3D{width, height, 1u};
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = vk::SampleCountFlagBits::e1;
		info.tiling = vk::ImageTiling::eOptimal;
		info.usage = usage;
		info.sharingMode = vk::SharingMode::eExclusive;
		info.initialLayout = vk::ImageLayout::eUndefined;

		m_layout = info.initialLayout;
		m_format = info.format;
		genImage(allocator, allocCreateInfo, info);
		genImageView();
	}

	Image::~Image()
	{
		if (m_allocator && m_image && m_allocation)
			vmaDestroyImage(m_allocator, m_image, m_allocation);
	}

	Image::Image(const Image& other)
	{
		*this = other;
	}

	Image::Image(Image&& other) noexcept
	{
		*this = std::move(other);
	}

	Image& Image::operator=(const Image& other)
	{
		if (!other.m_allocator) // other is default constructed, default construct on this
		{
			*this = Image();
			return *this;
		}

		VmaAllocationCreateInfo allocCreateInfo{};
		uint32_t index = other.memoryTypeIndex();
		allocCreateInfo.memoryTypeBits = 1u << index;

		// Call move assignment operator. After swapping, already exist object get destructed in temp object.
		*this = Image(other.m_allocator, allocCreateInfo, other.m_width, other.m_height, other.m_usage);
		return *this;
	}

	Image& Image::operator=(Image&& other) noexcept
	{
		AllocationBase::operator=(std::move(other));
		using std::swap;
		swap(m_width, other.m_width);
		swap(m_height, other.m_height);
		swap(m_layout, other.m_layout);
		swap(m_image, other.m_image);
		swap(m_stagingBuffer, other.m_stagingBuffer);
		swap(m_imageView, other.m_imageView);
		swap(m_usage, other.m_usage);
		swap(m_sampleCount, other.m_sampleCount);
		return *this;
	}

	/**
	 * \brief Change layout of the image. Barrier block all commands.
	 */
	void Image::changeLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask)
	{
		if (newLayout == m_layout) return;

		vk::ImageMemoryBarrier2 barrier = createLayoutTransitionMemoryBarrier(newLayout, aspectMask);
		cb.pipelineBarrier2({{}, {}, {}, barrier});

		vk::ImageLayout oldLayout = m_layout;
		m_layout = newLayout;
	}

	vk::ImageMemoryBarrier2 Image::createLayoutTransitionMemoryBarrier(vk::ImageLayout newLayout,
	                                                                   vk::ImageAspectFlags aspectMask) const
	{
		vk::ImageMemoryBarrier2 barrier{
			vk::PipelineStageFlagBits2::eAllCommands,
			vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead,
			vk::PipelineStageFlagBits2::eAllCommands,
			vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead,
			m_layout, newLayout
		};
		barrier.image = m_image;
		barrier.subresourceRange = {aspectMask, 0, 1, 0, 1};
		
		return barrier;
	}

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
}
