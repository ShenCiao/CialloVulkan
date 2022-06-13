#pragma once
#include "Window.hpp"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include "vku.hpp"

namespace ciallo::vulkan
{
	/**
	 * \brief Main pass is render pass for rendering imgui, anything else would be rendered as texture into main pass(imgui).
	 * Warning: Imgui vulkan implementation takes 2ms to render himself on ShenCiao's laptop(RTX 3060). Meanwhile openGL 1ms and Direct12 0.5ms. It seems caused by not choosing optimal memory type for buffer and image(find functions calling `ImGui_ImplVulkan_MemoryType` in file imgui_impl_vulkan.cpp). Or using looped semaphore in FramePresent. May invoke performance issue.
	 */
	class MainPassRenderer
	{
	public:
		explicit MainPassRenderer(Window* const w);
		~MainPassRenderer();
		void init();
		void render(vk::CommandBuffer cb, uint32_t framebufferIndex, ImDrawData* drawData);
	private:
		static void imguiCheckVkResult(VkResult err);

		void initImGui();
		void createDescriptorPool();
		void createSyncObject();
		void createRenderPass();
		void uploadFonts();

	public:
		void createFramebuffers();
	private:
		Window* w;
		bool m_imguiInitialized = false;
		constexpr static int MAX_SIZE = 128;
		std::vector<vk::DescriptorPoolSize> m_descriptorPoolSizes{
			{vk::DescriptorType::eSampler, MAX_SIZE},
            {vk::DescriptorType::eCombinedImageSampler, MAX_SIZE},
            {vk::DescriptorType::eSampledImage, MAX_SIZE},
            {vk::DescriptorType::eStorageImage, MAX_SIZE},
            {vk::DescriptorType::eUniformTexelBuffer, MAX_SIZE},
			{vk::DescriptorType::eStorageTexelBuffer, MAX_SIZE},
            {vk::DescriptorType::eUniformBuffer, MAX_SIZE},
            {vk::DescriptorType::eStorageBuffer, MAX_SIZE},
            {vk::DescriptorType::eUniformBufferDynamic, MAX_SIZE},
            {vk::DescriptorType::eStorageBufferDynamic, MAX_SIZE},
            {vk::DescriptorType::eInputAttachment, MAX_SIZE},
		};
		vk::UniqueDescriptorPool m_descriptorPool;

		std::vector<vk::Semaphore> m_waitSemaphores{}; //Semaphores needed to wait from other renderers(e.g. scene renderer). Need add and remove manually.

	public:
		vk::UniqueFence m_renderingCompleteFence;
		vk::UniqueSemaphore m_renderingCompleteSemaphore;
		vk::UniqueRenderPass m_renderPass;
		std::vector<vk::UniqueFramebuffer> m_framebuffers;
	};
}