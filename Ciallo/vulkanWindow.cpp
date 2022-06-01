#include "pch.hpp"
#include "vulkanWindow.hpp"

ciallo::vulkan::Window::Window(int height, int width, const std::string& title)
{
	glfwSetErrorCallback(glfwErrorCallback);
	//TODO: move init outside
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	m_glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

ciallo::vulkan::Window::~Window()
{
	glfwDestroyWindow(m_glfwWindow);
	glfwTerminate();
}

void ciallo::vulkan::Window::setFramework(const std::shared_ptr<Framework>& framework)
{
	m_framework = framework;
}

void ciallo::vulkan::Window::createSurface()
{
	VkSurfaceKHR rawSurface;
	if (glfwCreateWindowSurface(*m_framework->m_instance, m_glfwWindow, nullptr, &rawSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
	m_surface = vk::UniqueSurfaceKHR(rawSurface, *m_framework->m_instance);
}

/**
 * \brief create swapchain, extent, image and image view;
 */
void ciallo::vulkan::Window::createSwapchain()
{
	auto surfaceFormats = m_framework->m_physicalDevice.getSurfaceFormatsKHR(*m_surface);
	for (auto surface : surfaceFormats)
	{
		std::cout << vk::to_string(surface.format) << std::endl;
		std::cout << vk::to_string(surface.colorSpace) << std::endl;
	}

	m_swapchainImageFormat = vk::Format::eB8G8R8A8Srgb;

	int width, height;
	glfwGetFramebufferSize(m_glfwWindow, &width, &height);
	m_swapchainExtent = vk::Extent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

	vk::SwapchainCreateInfoKHR ci{
		{},
		*m_surface,
		3,
		m_swapchainImageFormat,
		vk::ColorSpaceKHR::eSrgbNonlinear,
		m_swapchainExtent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive
	};
	ci.presentMode = vk::PresentModeKHR::eMailbox;
	ci.clipped = VK_TRUE;

	//WARNING: ignore querying device support for swapchain extent, present mode, image formats and device's pre transform.
	m_swapchain = m_framework->m_device->createSwapchainKHRUnique(ci);
	m_swapchainImages = m_framework->m_device->getSwapchainImagesKHR(*m_swapchain);
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
		return m_framework->m_device->createImageViewUnique(ci);
	};
	m_swapchainImageViews = m_swapchainImages | views::transform(image2imageView) | ranges::to_vector;
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
	m_renderPass = rpm.createUnique(*m_framework->m_device);
}

void ciallo::vulkan::Window::createFramebuffers()
{
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
		return m_framework->m_device->createFramebufferUnique(info);
	};
	m_framebuffers = m_swapchainImageViews | views::transform(imageView2framebuffer) | ranges::to_vector;
}

void ciallo::vulkan::Window::recreateOnWindowResize()
{
	// m_framework->m_device->waitIdle();

	createSwapchain();
}

std::vector<vk::UniqueCommandBuffer> ciallo::vulkan::Window::createCommandBuffers(
	const vk::CommandBufferLevel level, const int n) const
{
	vk::CommandBufferAllocateInfo info{
		*m_framework->m_commandPool,
		level,
		static_cast<uint32_t>(n)
	};
	return m_framework->m_device->allocateCommandBuffersUnique(info);
}
