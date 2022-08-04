#include "pch.hpp"
#include "Image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ciallo::vulkan
{
	Image::Image(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo, vk::ImageCreateInfo info):
		AllocationBase(allocator), m_format(info.format), m_extent(info.extent),
		m_sampleCount(info.samples), m_usage(info.usage), m_layout(info.initialLayout)
	{
		auto viewType = imageTypeToImageViewType(info.imageType);
		m_image = createImage(allocator, allocCreateInfo, info);
		m_imageView = createImageView(m_image, viewType, info.format);
	}

	Image::Image(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo,
	             vk::Format format, uint32_t width, uint32_t height,
	             vk::SampleCountFlagBits sampleCount, vk::ImageUsageFlags usage):
		AllocationBase(allocator), m_format(format), m_extent({width, height, 1u}),
		m_sampleCount(sampleCount), m_usage(usage)
	{
		vk::ImageCreateInfo info{};
		info.imageType = vk::ImageType::e2D;
		info.format = format;
		info.extent = vk::Extent3D{width, height, 1u};
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = sampleCount;
		info.tiling = vk::ImageTiling::eOptimal;
		info.usage = usage;
		info.sharingMode = vk::SharingMode::eExclusive;
		info.initialLayout = vk::ImageLayout::eUndefined;

		auto viewType = imageTypeToImageViewType(info.imageType);

		m_layout = info.initialLayout;
		m_image = createImage(allocator, allocCreateInfo, info);
		m_imageView = createImageView(m_image, viewType, format);
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
		*this = Image(other.m_allocator, allocCreateInfo, other.m_format,
		              other.m_extent.width, other.m_extent.height, other.m_sampleCount, other.m_usage);
		return *this;
	}

	Image& Image::operator=(Image&& other) noexcept
	{
		AllocationBase::operator=(std::move(other));
		using std::swap;
		swap(m_format, other.m_format);
		swap(m_extent, other.m_extent);
		swap(m_sampleCount, other.m_sampleCount);
		swap(m_usage, other.m_usage);
		swap(m_layout, other.m_layout);
		swap(m_image, other.m_image);
		swap(m_stagingBuffer, other.m_stagingBuffer);
		swap(m_imageView, other.m_imageView);
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
		if (size == VK_WHOLE_SIZE) size = this->size();
		if (hostVisible())
		{
			uploadLocal(data, size);
			return;
		}
		if (!m_stagingBuffer)
		{
			genStagingBuffer();
		}
		m_stagingBuffer->uploadLocal(data, size);
		uploadStaging(cb, data, size, *m_stagingBuffer);
	}

	void Image::uploadLocal(const void* data, vk::DeviceSize size) const
	{
		if (size == VK_WHOLE_SIZE) size = this->size();
		AllocationBase::uploadLocal(data, 0, size);
	}

	vk::DeviceSize Image::size() const
	{
		vk::DeviceSize pixelSize = vk::blockSize(m_format);
		return m_extent.width * m_extent.height * m_extent.depth * pixelSize;
	}

	void Image::genStagingBuffer()
	{
		VmaAllocationCreateInfo info{VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO};
		m_stagingBuffer = std::make_unique<Buffer>(m_allocator, info, size(), vk::BufferUsageFlagBits::eTransferSrc);
	}

	vk::ImageViewType Image::imageTypeToImageViewType(vk::ImageType imType)
	{
		using vk::ImageType;
		using vk::ImageViewType;
		switch (imType)
		{
		case ImageType::e1D: return ImageViewType::e1D;
		case ImageType::e2D: return ImageViewType::e2D;
		case ImageType::e3D: return ImageViewType::e3D;
		}
		return {};
	}

	vk::UniqueImageView Image::createImageView(vk::Image image, vk::ImageViewType viewType, vk::Format format) const
	{
		vk::ImageViewCreateInfo info{};
		info.setImage(image);
		info.setViewType(viewType);
		info.setFormat(format);
		info.setComponents({});
		info.setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
		return device().createImageViewUnique(info);
	}

	// Only color image for now.
	void Image::uploadStaging(vk::CommandBuffer cb, const void* data, vk::DeviceSize size,
	                          vk::Buffer stagingBuffer) const
	{
		// Refer to vulkan spec struct VkImageSubresourceRange for aspectMask information
		vk::BufferImageCopy copy{};
		copy.setBufferOffset(0);
		copy.setImageExtent(m_extent);
		copy.setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});
		cb.copyBufferToImage(stagingBuffer, m_image, m_layout, copy);
	}

	vk::Image Image::createImage(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo,
	                             vk::ImageCreateInfo info)
	{
		auto i = static_cast<VkImageCreateInfo>(info);
		VkImage image;
		vmaCreateImage(allocator, &i, &allocCreateInfo, &image, &m_allocation, nullptr);
		return image;
	}
}
