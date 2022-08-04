#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "Device.hpp"
#include "Instance.hpp"

namespace ciallo::vulkan
{
	/**
	 * \brief Class wraps up glfw. Include windows object and swapchain related object.
	 * Glfw will be changed into platform native window api or something else. Since the imgui_impl_glfw(for vulkan) introduce extra lag and supporting tablet and stylus is a rabbit hole.
	 * 
	 */
	class Window
	{
		friend class MainPassRenderer;

		GLFWwindow* m_glfwWindow;
		vk::Instance m_instance;
		vk::Device m_device;
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueSurfaceKHR m_surface;
		vk::UniqueSwapchainKHR m_swapchain;
		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::UniqueImageView> m_swapchainImageViews;
		vk::Extent2D m_swapchainExtent;
		vk::Format m_swapchainImageFormat;
		vk::ColorSpaceKHR m_swapchainImageColorSpace;

	public:
		explicit Window(const std::string& title, bool visible = false);
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

		vk::Device device() const;

	public:
		void imguiInitWindow() const;
		void imguiShutdownWindow();
		void imguiNewFrame();
		void initSwapchain();
	public:
		void setInstance(vk::Instance instance);
		void setDevice(vk::Device device);
		void setPhysicalDevice(vk::PhysicalDevice physicalDevice);

		vk::SurfaceKHR genSurface();
		void genSwapchainFormat();
		void genSwapchain();
	public:
		void onWindowResize();

		void show() const;
		void hide() const;
		bool shouldClose() const;
		void pollEvents() const;
		static std::vector<const char*> getRequiredInstanceExtensions();

	public:
		vk::SurfaceKHR surface() const;
		vk::SwapchainKHR swapchain() const;
		vk::Extent2D swapchainExtent() const;
		int swapchainImageCount() const;
		vk::Instance instance() const;
		vk::Format swapchainImageFormat() const;
	};
}
