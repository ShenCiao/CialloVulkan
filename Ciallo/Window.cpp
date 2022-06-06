#include "pch.hpp"
#include "Window.hpp"

ciallo::vulkan::Window::Window(int height, int width, const std::string& title, bool visible)
{
	glfwSetErrorCallback(glfwErrorCallback);
	glfwInit();
	glfwWindowHint(GLFW_VISIBLE, visible);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	m_glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

ciallo::vulkan::Window::~Window()
{
	glfwDestroyWindow(m_glfwWindow);
	glfwTerminate();
}

void ciallo::vulkan::Window::pickQueueFamily()
{
	auto queueFamilies = m_physicalDevice.getQueueFamilyProperties();

	for (const auto [i, queueFamily] : views::enumerate(queueFamilies))
	{
		if (queueFamily.queueCount > 0 &&
			queueFamily.queueFlags & vk::QueueFlagBits::eGraphics &&
			queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
		{
			m_queueFamilyIndex = static_cast<uint32_t>(i);
			return;
		}
	}
	throw std::runtime_error("no required queue in physical device");
}

void ciallo::vulkan::Window::createDevice()
{
	vku::DeviceMaker dm{};
	dm.queue(m_queueFamilyIndex, 1.0f);
	for (const char* ext : m_deviceExtensions)
	{
		dm.extension(ext);
	}
	dm.extension(m_deviceExtensions[0]);
	m_device = dm.createUnique(m_physicalDevice);
}

void ciallo::vulkan::Window::createCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	poolInfo.queueFamilyIndex = m_queueFamilyIndex;
	m_commandPool = m_device->createCommandPoolUnique(poolInfo);
}

void ciallo::vulkan::Window::createPhysicalDevice(int index)
{
	auto devices = m_instance->m_instance->enumeratePhysicalDevices();
	m_physicalDevice = devices[index];
}

void ciallo::vulkan::Window::setInstance(const std::shared_ptr<Instance>& instance)
{
	m_instance = instance;
}

void ciallo::vulkan::Window::createSurface()
{
	VkSurfaceKHR rawSurface;
	if (glfwCreateWindowSurface(*m_instance->m_instance, m_glfwWindow, nullptr, &rawSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
	m_surface = vk::UniqueSurfaceKHR(rawSurface, *m_instance->m_instance);
}

void ciallo::vulkan::Window::pickSurfaceFormat()
{
	//TODO: make it portable
	auto surfaceFormats = m_physicalDevice.getSurfaceFormatsKHR(*m_surface);
	for (auto surface : surfaceFormats)
	{
		std::cout << vk::to_string(surface.format) << std::endl;
		std::cout << vk::to_string(surface.colorSpace) << std::endl;
	}
	m_swapchainImageFormat = vk::Format::eB8G8R8A8Srgb;
	m_swapchainImageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
}

/**
 * \brief create swapchain and its extent, images, image views, framebuffers;
 */
void ciallo::vulkan::Window::createSwapchain()
{
	int width, height;
	glfwGetFramebufferSize(m_glfwWindow, &width, &height);
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
	ci.presentMode = vk::PresentModeKHR::eMailbox;
	ci.clipped = VK_TRUE;
	ci.oldSwapchain = *m_swapchain;

	//WARNING: ignore querying device support for swapchain extent, present mode, image formats and device's pre transform.
	m_swapchain = m_device->createSwapchainKHRUnique(ci);
	m_swapchainImages = m_device->getSwapchainImagesKHR(*m_swapchain);
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
		return m_device->createImageViewUnique(ci);
	};
	m_swapchainImageViews = m_swapchainImages | views::transform(image2imageView) | ranges::to_vector;

	auto imageView2framebuffer = [this](const vk::UniqueImageView& imv)
	{
		std::vector<vk::ImageView> views{*imv};
		vk::FramebufferCreateInfo info{
			{},
			*m_renderPass,
			views,
			m_swapchainExtent.width,
			m_swapchainExtent.height,
			1
		};
		return m_device->createFramebufferUnique(info);
	};
	m_swapchainFramebuffers = m_swapchainImageViews | views::transform(imageView2framebuffer) | ranges::to_vector;
}

void ciallo::vulkan::Window::createRenderpass()
{
	vku::RenderpassMaker rpm;
	rpm.attachmentBegin(m_swapchainImageFormat)
	   .attachmentLoadOp(vk::AttachmentLoadOp::eClear)
	   .attachmentStoreOp(vk::AttachmentStoreOp::eStore)
	   .attachmentFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	rpm.subpassBegin(vk::PipelineBindPoint::eGraphics)
	   .subpassColorAttachment(vk::ImageLayout::eAttachmentOptimal, 0);
	m_renderPass = rpm.createUnique(*m_device);
}

void ciallo::vulkan::Window::onWindowResize()
{
	m_device->waitIdle();

	createSwapchain();
}

std::vector<vk::UniqueCommandBuffer> ciallo::vulkan::Window::createCommandBuffers(
	const vk::CommandBufferLevel level, const int n) const
{
	vk::CommandBufferAllocateInfo info{
		*m_commandPool,
		level,
		static_cast<uint32_t>(n)
	};
	return m_device->allocateCommandBuffersUnique(info);
}

bool ciallo::vulkan::Window::isPhysicalDeviceValid(vk::PhysicalDevice device)
{
		// need a queue family be able to graphics, compute and presents(unchecked)
	auto queueFamilies = device.getQueueFamilyProperties();
	bool queueFamilyFound = false;
	for (const auto [i, queueFamily] : views::enumerate(queueFamilies))
	{
		if (queueFamily.queueCount > 0 &&
			queueFamily.queueFlags & vk::QueueFlagBits::eGraphics &&
			queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
		{
			queueFamilyFound = true;
			break;
		}
	}

	// device extension support
	auto extensions = device.enumerateDeviceExtensionProperties();
	std::unordered_set<std::string> requiredExtensions{m_deviceExtensions.begin(), m_deviceExtensions.end()};

	for (const auto& extension : extensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}
	bool extensionsFound = requiredExtensions.empty();


	// may need check swapchain
	return queueFamilyFound && extensionsFound;
}
