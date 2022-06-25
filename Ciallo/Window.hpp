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
		friend class MainPassRenderer;

		vk::UniqueDevice m_device;
		vk::PhysicalDevice m_physicalDevice;
		uint32_t m_queueFamilyIndex;
		vk::UniqueCommandPool m_commandPool;
		std::vector<const char*> m_deviceExtensions{
			"VK_KHR_swapchain"
		};

		GLFWwindow* m_glfwWindow;
		std::shared_ptr<Instance> m_instance;
		vk::UniqueSurfaceKHR m_surface;
		vk::UniqueSwapchainKHR m_swapchain;
		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::UniqueImageView> m_swapchainImageViews;
		vk::Extent2D m_swapchainExtent;
		vk::Format m_swapchainImageFormat;
		vk::ColorSpaceKHR m_swapchainImageColorSpace;
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
		void imguiInitWindow() const;
		void imguiShutdownWindow();
		void imguiNewFrame();
		void initResources();
	public:
		void pickPhysicalDevice(int index);
		// supposed to be private
		void setInstance(const std::shared_ptr<Instance>& instance);
		void pickQueueFamily();
		void genDevice();
		void genCommandPool();
		void genSurface();
		void pickSurfaceFormat();
		void genSwapchain();
	public:
		bool isPhysicalDeviceValid(vk::PhysicalDevice device);
		void onWindowResize();
		std::vector<vk::UniqueCommandBuffer>
		createCommandBuffers(vk::CommandBufferLevel level, uint32_t n) const;

	public:
		void show() const;
		void hide() const;
		bool shouldClose() const;
		void pollEvents() const;
		std::vector<const char*> getRequiredInstanceExtensions() const;
		void executeImmediately(const std::function<void(vk::CommandBuffer)>& func);

	public:
		vk::SwapchainKHR swapchain() const;
		vk::Extent2D swapchainExtent() const;
		vk::Device device() const;
		int swapchainImageCount() const;
		vk::Queue queue() const;
		vk::Instance instance() const;
		vk::PhysicalDevice physicalDevice() const;
		uint32_t queueFamilyIndex() const;
		vk::Format swapchainImageFormat() const;
	};
}
