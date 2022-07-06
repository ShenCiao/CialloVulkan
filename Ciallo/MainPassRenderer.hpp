#pragma once
#include "Window.hpp"
#include "Device.hpp"

#include <imgui_impl_vulkan.h>

namespace ciallo::vulkan
{
	/**
	 * \brief Main pass is a render pass targeting on framebuffer created from swapchain image. Currently only for rendering imgui, anything else would be rendered as textures into imgui.
	 * Warning: Imgui vulkan implementation takes 2ms to render himself on ShenCiao's laptop(RTX 3060, AMD 4800H) in immediate mode. Meanwhile openGL 1ms and Direct12 0.5ms. It seems caused by not choosing optimal memory type for buffer and image(find functions calling `ImGui_ImplVulkan_MemoryType` in file imgui_impl_vulkan.cpp). Or using looped semaphore in function FramePresent. May invoke performance issue.
	 */
	class MainPassRenderer
	{
	private:
		Window* w;
		Device* d;
		bool m_imguiInitialized = false;

		//Semaphores needed to wait from other renderers(e.g. scene renderer). Need add and remove manually. [Unused for now].
		std::vector<vk::Semaphore> m_waitSemaphores{};
		vk::UniqueFence m_renderingCompleteFence;
		vk::UniqueSemaphore m_renderingCompleteSemaphore;
		vk::UniqueRenderPass m_renderPass;
		std::vector<vk::UniqueFramebuffer> m_framebuffers;
	public:
		explicit MainPassRenderer(Window* const w);
		MainPassRenderer(const MainPassRenderer& other) = delete;
		MainPassRenderer(MainPassRenderer&& other) = default;
		MainPassRenderer& operator=(const MainPassRenderer& other) = delete;
		MainPassRenderer& operator=(MainPassRenderer&& other) = default;
		~MainPassRenderer();

		void init();
		void render(vk::CommandBuffer cb, uint32_t framebufferIndex, ImDrawData* drawData);
		static void imguiCheckVkResult(VkResult err);
	private:
		void initImGui();
		void genSyncObject();
		void genRenderPass();
		void uploadFonts() const;

	public:
		void genFramebuffers();

	public:
		vk::Fence renderingCompleteFence() const
		{
			return *m_renderingCompleteFence;
		}

		vk::Semaphore renderingCompleteSemaphore() const
		{
			return *m_renderingCompleteSemaphore;
		}

		vk::RenderPass renderPass() const
		{
			return *m_renderPass;
		}
	};
}
