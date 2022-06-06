#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "Instance.hpp"

namespace ciallo::vulkan
{
	class Window
	{
	public:
		Window(int height, int width, const std::string& title, bool visible = false);
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
		void setInstance(const std::shared_ptr<Instance>& instance);
		void createPhysicalDevice(int index);
		void pickQueueFamily();
		void createDevice();
		void createCommandPool();
		void createSurface();
		void pickSurfaceFormat();
		void createSwapchain();
		void createRenderpass();
		void onWindowResize();
		std::vector<vk::UniqueCommandBuffer>
		createCommandBuffers(const vk::CommandBufferLevel level, const int n) const;
		bool isPhysicalDeviceValid(vk::PhysicalDevice device);

	public:
		void show() const { glfwShowWindow(m_glfwWindow); }
		void hide() const { glfwHideWindow(m_glfwWindow); }
		bool shouldClose() const { return glfwWindowShouldClose(m_glfwWindow); }
	private:
		GLFWwindow* m_glfwWindow;
		std::shared_ptr<Instance> m_instance;
		std::vector<const char*> m_deviceExtensions{
			"VK_KHR_swapchain"
		};

	private:
		uint32_t m_queueFamilyIndex;
		vk::UniqueDevice m_device;
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueCommandPool m_commandPool;
		vk::UniqueSurfaceKHR m_surface;
		vk::UniqueSwapchainKHR m_swapchain;
		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::UniqueImageView> m_swapchainImageViews;
		std::vector<vk::UniqueFramebuffer> m_swapchainFramebuffers;
		vk::Extent2D m_swapchainExtent;
		vk::Format m_swapchainImageFormat;
		vk::ColorSpaceKHR m_swapchainImageColorSpace;
		vk::UniqueRenderPass m_renderPass;

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
			return *m_device;
		}

		int swapchainImageCount() const
		{
			return static_cast<int>(m_swapchainImages.size());
		}

		std::vector<vk::Framebuffer> framebuffers() const
		{
			std::vector<vk::Framebuffer> v;
			for (const auto& fb : m_swapchainFramebuffers)
			{
				v.push_back(*fb);
			}
			return v;
		}

		vk::Framebuffer framebuffer(const int index) const
		{
			return *m_swapchainFramebuffers[index];
		}

		vk::Queue queue() const
		{
			return m_device->getQueue(m_queueFamilyIndex, 0);
		}
	};
}
