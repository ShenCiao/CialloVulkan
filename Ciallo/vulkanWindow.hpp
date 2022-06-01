#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "vulkanFramework.hpp"

namespace ciallo::vulkan
{
	class Window
	{
	public:
		Window(int height, int width, const std::string& title);
		~Window();

		Window(const Window& other) = delete;
		Window(Window&& other) = default;
		Window& operator=(const Window& other) = delete;
		Window& operator=(Window&& other) = default;

	private:
		static void glfwErrorCallback(int error, const char* description)
		{
			spdlog::error("Glfw error: {}", description);
		}

	public:
		void setFramework(const std::shared_ptr<Framework>& framework);
		void createSurface();
		void createSwapchain();
		void createRenderpass();
		void createFramebuffers();

		void recreateOnWindowResize();
		std::vector<vk::UniqueCommandBuffer>
		createCommandBuffers(const vk::CommandBufferLevel level, const int n) const;

		bool shouldClose() const { return glfwWindowShouldClose(m_glfwWindow); }
	private:
		GLFWwindow* m_glfwWindow; //self-owned

		std::shared_ptr<Framework> m_framework;

	private:
		vk::UniqueSurfaceKHR m_surface;
		vk::UniqueSwapchainKHR m_swapchain;
		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::UniqueImageView> m_swapchainImageViews;
		std::vector<vk::Framebuffer> m_swapchainFramebuffers;
		vk::Extent2D m_swapchainExtent;
		vk::Format m_swapchainImageFormat;
		vk::UniqueRenderPass m_renderPass;
		std::vector<vk::UniqueFramebuffer> m_framebuffers;
	public:
		vk::SwapchainKHR swapchain() const
		{
			return *m_swapchain;
		}

		vk::RenderPass renderPass() const
		{
			return *m_renderPass;
		}

		vk::Extent2D swapchainExtent() const
		{
			return m_swapchainExtent;
		}

		vk::Device device() const
		{
			return *m_framework->m_device;
		}

		int swapchainImageCount() const
		{
			return static_cast<int>(m_swapchainImages.size());
		}

		std::vector<vk::Framebuffer> framebuffers() const
		{
			std::vector<vk::Framebuffer> v;
			for (const auto& fb : m_framebuffers)
			{
				v.push_back(*fb);
			}
			return v;
		}

		vk::Framebuffer framebuffer(const int index) const
		{
			return *m_framebuffers[index];
		}

		vk::Queue queue() const
		{
			return m_framework->m_device->getQueue(m_framework->m_queueFamilyIndex, 0);
		}
	};
}
