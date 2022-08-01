#include "pch.hpp"
#include "Mapper.hpp"

namespace ciallo::vulkan
{
	Mapper::Mapper(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo, vk::ImageCreateInfo info)
		: Image(allocator, allocCreateInfo, info)
	{
	}

	Mapper::Mapper(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo, vk::ImageType inType,
	               vk::Format outType, vk::Extent3D extent, vk::ImageUsageFlags usage):
		Image(allocator, allocCreateInfo,
		      {{}, inType, outType, extent, 1u, 1u, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usage}),
		imageType(inType)
	{
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
}
