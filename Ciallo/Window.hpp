#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "Instance.hpp"

namespace ciallo::vulkan
{
	/**
	 * \brief Wrap up glfw and vulkan infrastructure for ciallo, will be changed into platform native api or something else.
	 *	Since the imgui_impl_glfw(for vulkan) introduce extra lag and supporting tablet and stylus is a rabbit hole.
	 */
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
		void imguiInitWindow();
		void imguiShutdownWindow();
		void initResources();
	private:
		void setInstance(const std::shared_ptr<Instance>& instance);
		void createPhysicalDevice(int index);
		void pickQueueFamily();
		void createDevice();
		void createCommandPool();
		void createSurface();
		void pickSurfaceFormat();
		void createSwapchain();
		void createRenderPass();
		void onWindowResize();
		std::vector<vk::UniqueCommandBuffer>
		createCommandBuffers(const vk::CommandBufferLevel level, const int n) const;
		bool isPhysicalDeviceValid(vk::PhysicalDevice device);

	public:
		void show() const;
		void hide() const;
		bool shouldClose() const;
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
		vk::SwapchainKHR swapchain() const;
		vk::RenderPass renderPass() const;
		vk::Extent2D swapchainExtent() const;
		vk::Device device() const;
		int swapchainImageCount() const;
		std::vector<vk::Framebuffer> swapchainFramebuffers() const;
		vk::Framebuffer framebuffer(const int index) const;
		vk::Queue queue() const;
		vk::Instance instance();
		vk::PhysicalDevice physicalDevice();
		uint32_t queueFamilyIndex();
	};
}
