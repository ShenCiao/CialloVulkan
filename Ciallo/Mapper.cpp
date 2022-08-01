#include "pch.hpp"
#include "Mapper.hpp"

namespace ciallo::vulkan
{
	Mapper::Mapper(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo, vk::ImageCreateInfo info)
	{
		m_allocator = allocator;
		m_format = info.format;
		m_extent = info.extent;
		m_sampleCount = info.samples;
		m_usage = info.usage;
		m_layout = info.initialLayout;

		m_image = createImage(allocator, allocCreateInfo, info);
		m_imageView = createImageView();
	}

	Mapper::Mapper(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo, vk::ImageType inType,
	               vk::Format outType, vk::Extent3D extent, vk::ImageUsageFlags usage): imageType(inType)
	{
		vk::ImageCreateInfo info{{}, inType, outType, extent, 1u, 1u};
		info.setUsage(usage);
		m_image = createImage(allocator, allocCreateInfo, info);
		m_imageView = createImageView();
	}

	Mapper::~Mapper()
	{
		if (m_allocator && m_image && m_allocation)
			vmaDestroyImage(m_allocator, m_image, m_allocation);
	}

	Mapper::Mapper(const Mapper& other)
	{
		*this = other;
	}

	Mapper::Mapper(Mapper&& other) noexcept
	{
		*this = std::move(other);
	}

	Mapper& Mapper::operator=(const Mapper& other)
	{
		if (!other.m_allocator) // other is default constructed, default construct on this
		{
			*this = Mapper();
			return *this;
		}

		VmaAllocationCreateInfo allocCreateInfo{};
		uint32_t index = other.memoryTypeIndex();
		allocCreateInfo.memoryTypeBits = 1u << index;

		// Call move assignment operator. After swapping, already exist object get destructed in temp object.
		*this = Mapper(other.m_allocator, allocCreateInfo, other.imageType,
		               other.m_format, other.m_extent, other.m_usage);
		return *this;
	}

	Mapper& Mapper::operator=(Mapper&& other) noexcept
	{
		Image::operator=(std::move(other));
		using std::swap;
		swap(imageType, other.imageType);
		return *this;
	}

	vk::UniqueImageView Mapper::createImageView()
	{
		vk::ImageViewCreateInfo info{};
		info.setImage(m_image)
		    .setViewType(imageTypeToImageViewType(imageType))
		    .setFormat(m_format)
		    .setComponents({})
		    .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
		return device().createImageViewUnique(info);
	}

	vk::ImageViewType Mapper::imageTypeToImageViewType(vk::ImageType imType)
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
}
