#include "pch.hpp"
#include "Image.hpp"

namespace ciallo::vulkan
{
	Image::Image(VmaAllocator allocator, const std::string& path, vk::ImageUsageFlags usage)
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
		m_mipLevels = 1;
		m_arrayLayers = 1;

		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = static_cast<VkFormat>(m_format);
		info.extent.width = m_width;
		info.extent.height = m_height;
		info.extent.depth = 1;
		info.mipLevels = m_mipLevels;
		info.arrayLayers = m_arrayLayers;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = static_cast<VkImageUsageFlags>(usage);
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

		vmaCreateImage(allocator, &info, &allocInfo, &m_vkImage, &m_allocation, nullptr);
	}

	Image::~Image()
	{
		vmaDestroyImage(m_allocator, m_vkImage, m_allocation);
	}

	void Image::setLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask)
	{
		if (newLayout == m_layout) return;
		vk::ImageLayout oldLayout = m_layout;
		m_layout = newLayout;

		vk::ImageMemoryBarrier imageMemoryBarriers = {};
		imageMemoryBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarriers.oldLayout = oldLayout;
		imageMemoryBarriers.newLayout = newLayout;
		imageMemoryBarriers.image = m_vkImage;
		imageMemoryBarriers.subresourceRange = {aspectMask, 0, m_mipLevels, 0, m_arrayLayers};

		// Put barrier on top
		// https://www.khronos.org/registry/vulkan/specs/1.2/html/chap7.html#synchronization-access-types-supported
		vk::PipelineStageFlags srcStageMask{vk::PipelineStageFlagBits::eTopOfPipe};
		vk::PipelineStageFlags dstStageMask{vk::PipelineStageFlagBits::eTopOfPipe};
		vk::DependencyFlags dependencyFlags{};
		vk::AccessFlags srcMask{};
		vk::AccessFlags dstMask{};

		typedef vk::ImageLayout il;
		typedef vk::AccessFlagBits afb;

		// Is it me, or are these the same?
		switch (oldLayout)
		{
		case il::eUndefined: break;
		case il::eGeneral:
			srcMask = afb::eTransferWrite;
			srcStageMask = vk::PipelineStageFlagBits::eTransfer;
			break;
		case il::eColorAttachmentOptimal:
			srcMask = afb::eColorAttachmentWrite;
			srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			break;
		case il::eDepthStencilAttachmentOptimal:
			srcMask = afb::eDepthStencilAttachmentWrite;
			srcStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
			break;
		case il::eDepthStencilReadOnlyOptimal:
			srcMask = afb::eDepthStencilAttachmentRead;
			srcStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
			break;
		case il::eShaderReadOnlyOptimal:
			srcMask = afb::eShaderRead;
			srcStageMask = vk::PipelineStageFlagBits::eVertexShader;
			break;
		case il::eTransferSrcOptimal:
			srcMask = afb::eTransferRead;
			srcStageMask = vk::PipelineStageFlagBits::eTransfer;
			break;
		case il::eTransferDstOptimal:
			srcMask = afb::eTransferWrite;
			srcStageMask = vk::PipelineStageFlagBits::eTransfer;
			break;
		case il::ePreinitialized:
			srcMask = afb::eTransferWrite | afb::eHostWrite;
			srcStageMask = vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eHost;
			break;
		case il::ePresentSrcKHR:
			srcMask = afb::eMemoryRead;
			break;
		}

		switch (newLayout)
		{
		case il::eUndefined: break;
		case il::eGeneral:
			dstMask = afb::eTransferWrite;
			dstStageMask = vk::PipelineStageFlagBits::eTransfer;
			break;
		case il::eColorAttachmentOptimal:
			dstMask = afb::eColorAttachmentWrite;
			dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			break;
		case il::eDepthStencilAttachmentOptimal:
			dstMask = afb::eDepthStencilAttachmentWrite;
			dstStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
			break;
		case il::eDepthStencilReadOnlyOptimal:
			dstMask = afb::eDepthStencilAttachmentRead;
			dstStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
			break;
		case il::eShaderReadOnlyOptimal:
			dstMask = afb::eShaderRead;
			dstStageMask = vk::PipelineStageFlagBits::eVertexShader;
			break;
		case il::eTransferSrcOptimal:
			dstMask = afb::eTransferRead;
			dstStageMask = vk::PipelineStageFlagBits::eTransfer;
			break;
		case il::eTransferDstOptimal:
			dstMask = afb::eTransferWrite;
			dstStageMask = vk::PipelineStageFlagBits::eTransfer;
			break;
		case il::ePreinitialized:
			dstMask = afb::eTransferWrite;
			dstStageMask = vk::PipelineStageFlagBits::eTransfer;
			break;
		case il::ePresentSrcKHR:
			dstMask = afb::eMemoryRead;
			break;
		}

		imageMemoryBarriers.srcAccessMask = srcMask;
		imageMemoryBarriers.dstAccessMask = dstMask;
		auto memoryBarriers = nullptr;
		auto bufferMemoryBarriers = nullptr;
		cb.pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags, memoryBarriers, bufferMemoryBarriers,
		                   imageMemoryBarriers);
	}
}
