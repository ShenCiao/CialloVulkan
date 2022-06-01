#include "pch.hpp"
#include "Application.hpp"
#include "vulkanTest.hpp"

void ciallo::Application::run()
{
	// m_mainWindow = std::make_unique<vulkan::Window>(m_mainWindowWidth, m_mainWindowHeight, "Ciallo");
	// while(!m_mainWindow->shouldClose())
	// {
	// 	glfwPollEvents();
	//
	// }
	auto w = std::make_unique<vulkan::Window>(1000, 1000, "hi");
	auto fw = std::make_shared<vulkan::Framework>();
	uint32_t extensionsCount;
	const char** extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
	std::vector<const char*> glfwExtensions{extensions, extensions+extensionsCount};
	fw->addInstanceExtensions(glfwExtensions);
	fw->createInstance();
	fw->createPhysicalDevice(1);
	fw->pickQueueFamily();
	fw->createDevice();
	fw->createCommandPool();

	w->setFramework(fw);
	w->createSurface();
	w->createSwapchain();
	w->createRenderpass();
	w->createFramebuffers();

	vulkan::Test t(w.get());
}

void ciallo::Application::loadSettings()
{
	m_mainWindowWidth = 400;
	m_mainWindowHeight = 400;
}
