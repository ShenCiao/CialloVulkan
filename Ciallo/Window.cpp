﻿#include "pch.hpp"
#include "Window.hpp"

#include <imgui_impl_glfw.h>

namespace ciallo::vulkan
{
	Window::Window(int height, int width, const std::string& title, bool visible)
	{
		glfwSetErrorCallback(glfwErrorCallback);
		glfwInit();
		glfwWindowHint(GLFW_VISIBLE, visible);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		m_glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_glfwWindow);
		glfwTerminate();
	}

	void Window::imguiInitWindow() const
	{
		ImGui_ImplGlfw_InitForVulkan(m_glfwWindow, true);
	}

	void Window::imguiShutdownWindow()
	{
		ImGui_ImplGlfw_Shutdown();
	}

	void Window::imguiNewFrame()
	{
		ImGui_ImplGlfw_NewFrame();
	}

	void Window::initResources()
	{
		genSurface();
		pickSurfaceFormat();
		genSwapchain();
	}

	void Window::setInstance(const std::shared_ptr<Instance>& instance)
	{
		m_instance = instance;
	}

	void Window::setDevice(const std::shared_ptr<Device>& device)
	{
		m_device = device;
	}

	void Window::genSurface()
	{
		VkSurfaceKHR rawSurface;
		if (glfwCreateWindowSurface(*m_instance->m_instance, m_glfwWindow, nullptr, &rawSurface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
		m_surface = vk::UniqueSurfaceKHR(rawSurface, *m_instance->m_instance);
	}

	void Window::pickSurfaceFormat()
	{
		//TODO: make it portable, it's a dirty hack for ShenCiao's laptop only
		auto surfaceFormats = m_device->physicalDevice().getSurfaceFormatsKHR(*m_surface);
		for (auto surface : surfaceFormats)
		{
			std::cout << vk::to_string(surface.format) << std::endl;
			std::cout << vk::to_string(surface.colorSpace) << std::endl;
		}
		m_swapchainImageFormat = vk::Format::eB8G8R8A8Unorm;
		m_swapchainImageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	}

	/**
	 * \brief create swapchain and its extent, images, image views, framebuffers;
	 */
	void Window::genSwapchain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_glfwWindow, &width, &height);

		// When width is 0 or height is 0, window is being minimized or may opened just now.
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_glfwWindow, &width, &height);
			glfwWaitEvents();
		}

		m_swapchainExtent = vk::Extent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

		vk::SwapchainCreateInfoKHR ci{
			{},
			*m_surface,
			3,
			m_swapchainImageFormat,
			m_swapchainImageColorSpace,
			m_swapchainExtent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment,
			vk::SharingMode::eExclusive
		};
		ci.presentMode = vk::PresentModeKHR::eFifo;
		ci.clipped = VK_TRUE;
		ci.oldSwapchain = *m_swapchain;

		//WARNING: ignore querying device support for swapchain extent, present mode, image formats and device's pre transform.
		m_swapchain = device().createSwapchainKHRUnique(ci);
		m_swapchainImages = device().getSwapchainImagesKHR(*m_swapchain);
		spdlog::info("number of swapchain images: {}", m_swapchainImages.size());

		auto image2imageView = [this](const vk::Image im)
		{
			vk::ImageViewCreateInfo ci{};
			ci.image = im;
			ci.viewType = vk::ImageViewType::e2D;
			ci.format = m_swapchainImageFormat;
			ci.components.r = vk::ComponentSwizzle::eIdentity;
			ci.components.g = vk::ComponentSwizzle::eIdentity;
			ci.components.b = vk::ComponentSwizzle::eIdentity;
			ci.components.a = vk::ComponentSwizzle::eIdentity;
			ci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			ci.subresourceRange.baseMipLevel = 0;
			ci.subresourceRange.levelCount = 1;
			ci.subresourceRange.baseArrayLayer = 0;
			ci.subresourceRange.layerCount = 1;
			return device().createImageViewUnique(ci);
		};
		m_swapchainImageViews = m_swapchainImages | views::transform(image2imageView) | ranges::to_vector;
	}

	void Window::onWindowResize()
	{
		device().waitIdle();

		genSwapchain();
	}

	void Window::show() const
	{
		glfwShowWindow(m_glfwWindow);
	}

	void Window::hide() const
	{
		glfwHideWindow(m_glfwWindow);
	}

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(m_glfwWindow);
	}

	void Window::pollEvents() const
	{
		glfwPollEvents();
	}

	std::vector<const char*> Window::getRequiredInstanceExtensions()
	{
		uint32_t extensionsCount = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
		return {extensions, extensions + extensionsCount};
	}

	vk::SwapchainKHR Window::swapchain() const
	{
		return *m_swapchain;
	}

	vk::Extent2D Window::swapchainExtent() const
	{
		return m_swapchainExtent;
	}

	vk::Device Window::device() const
	{
		return *m_device;
	}

	int Window::swapchainImageCount() const
	{
		return static_cast<int>(m_swapchainImages.size());
	}


	vk::Instance Window::instance() const
	{
		return m_instance->instance();
	}


	vk::Format Window::swapchainImageFormat() const
	{
		return m_swapchainImageFormat;
	}
}
