#pragma once
#include "Image.hpp"

namespace ciallo::vulkan
{
	/**
	 * \brief Map variable inside shader
	 * Internally, it use sampler and image with color format. Maybe it's a bad idea to inherent from Image...
	 */
	class Mapper : public Image
	{
		vk::ImageType imageType{};

	public:
		Mapper() = default;
		Mapper(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo, vk::ImageCreateInfo info);
		Mapper(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo,
		       vk::ImageType inType, vk::Format outType, vk::Extent3D extent, vk::ImageUsageFlags usage);
		~Mapper();

		Mapper(const Mapper& other);
		Mapper(Mapper&& other) noexcept;
		Mapper& operator=(const Mapper& other);
		Mapper& operator=(Mapper&& other) noexcept;
	};
}
