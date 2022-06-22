#include "pch.hpp"
#include "ScenePanel.hpp"
#include <stb_image.h>
#include <imgui_impl_vulkan.h>
#include <filesystem>

namespace ciallo::gui
{
	void ScenePanel::draw() const
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_HorizontalScrollbar;
		ImGui::Begin("Scene", nullptr, flags);
		// ImGui::Image()
		ImVec2 sizeToShow{static_cast<float>(m_canvas->width()), static_cast<float>(m_canvas->height())};
		ImGui::Image(m_canvasTextureId, sizeToShow);
		ImGui::End();
	}

	void ScenePanel::createCanvas(VmaAllocator allocator, vk::CommandBuffer cb)
	{
		int width, height, channels;
		spdlog::info("Current working directory: {}", std::filesystem::current_path().string());
		
		auto data = reinterpret_cast<uint8_t*>(stbi_loadf("images/takagi3.png", &width, &height, &channels, 4));
		if (!data)
		{
			throw std::runtime_error("Unable to load image");
		}

		VmaAllocationCreateInfo info={};
		info.usage = VMA_MEMORY_USAGE_AUTO;
		m_canvas = std::make_unique<vulkan::Image>(allocator, info, width, height,
		                                           vk::ImageUsageFlagBits::eSampled |
		                                           vk::ImageUsageFlagBits::eTransferDst);
		auto barrier = m_canvas->genLayoutTransitionMemoryBarrier(vk::ImageLayout::eGeneral);

		cb.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {}, barrier);
		m_canvas->setLayout(vk::ImageLayout::eGeneral);
		m_canvas->upload(cb, data);
		m_canvasTextureId = ImGui_ImplVulkan_AddTexture(*m_sampler, m_canvas->imageView(), static_cast<VkImageLayout>(vk::ImageLayout::eGeneral));
	}

	void ScenePanel::createSampler(vk::Device device)
	{
		vk::SamplerCreateInfo info{};
        info.magFilter = vk::Filter::eLinear;
        info.minFilter = vk::Filter::eLinear;
        info.addressModeU = vk::SamplerAddressMode::eRepeat;
        info.addressModeV = vk::SamplerAddressMode::eRepeat;
        info.addressModeW = vk::SamplerAddressMode::eRepeat;
        info.anisotropyEnable = VK_FALSE;
        info.borderColor = vk::BorderColor::eIntOpaqueBlack;
        info.unnormalizedCoordinates = VK_FALSE;
        info.compareEnable = VK_FALSE;
        info.compareOp = vk::CompareOp::eAlways;
        info.mipmapMode = vk::SamplerMipmapMode::eLinear;
		m_sampler = device.createSamplerUnique(info);
	}
}
