﻿#include "pch.hpp"
#include "ScenePanel.hpp"

#include <filesystem>
#include <imgui_impl_vulkan.h>
#include <stb_image.h>

#include "Device.hpp"

namespace ciallo::gui
{
	void ScenePanel::draw() const
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar;
		ImGui::Begin("Scene", nullptr, flags);

		ImVec2 sizeToShow{static_cast<float>(m_canvas->width()), static_cast<float>(m_canvas->height())};
		ImGui::Image(m_canvasTextureId, sizeToShow);
		ImGui::End();
	}

	void ScenePanel::genCanvas(vulkan::Device* d, vk::CommandBuffer cb)
	{
		int width = 1024, height = 1024;
		VmaAllocationCreateInfo info = {{}, VMA_MEMORY_USAGE_AUTO};

		m_canvas = std::make_unique<vulkan::Image>(
			*d, info, vk::Format::eR8G8B8A8Unorm,
			width, height, vk::SampleCountFlagBits::e1,
			vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eTransferDst |
			vk::ImageUsageFlagBits::eColorAttachment);

		m_canvas->changeLayout(cb, vk::ImageLayout::eGeneral);

		m_canvasTextureId = ImGui_ImplVulkan_AddTexture(*m_sampler, m_canvas->imageView(),
		                                                static_cast<VkImageLayout>(m_canvas->imageLayout()));
	}

	void ScenePanel::genSampler(vk::Device device)
	{
		vk::SamplerCreateInfo info{};
		info.magFilter = vk::Filter::eNearest;
		info.minFilter = vk::Filter::eNearest;
		info.addressModeU = vk::SamplerAddressMode::eClampToBorder;
		info.addressModeV = vk::SamplerAddressMode::eClampToBorder;
		info.addressModeW = vk::SamplerAddressMode::eClampToBorder;
		info.anisotropyEnable = VK_FALSE;
		info.borderColor = vk::BorderColor::eIntOpaqueBlack;
		info.unnormalizedCoordinates = VK_FALSE;
		info.compareEnable = VK_FALSE;
		info.compareOp = vk::CompareOp::eAlways;
		m_sampler = device.createSamplerUnique(info);
	}
}
