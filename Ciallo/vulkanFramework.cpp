#include "pch.hpp"
#include "vulkanFramework.hpp"

ciallo::vulkan::Framework::~Framework()
{
	m_instance->destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_dldi);
}

VkBool32 ciallo::vulkan::Framework::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                  void* pUserData)
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		spdlog::warn("{}", pCallbackData->pMessage);
	}

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		spdlog::error("{}", pCallbackData->pMessage);
	}

	return VK_FALSE;
}

void ciallo::vulkan::Framework::createInstance()
{
	auto appInfo = vk::ApplicationInfo(
		"Ciallo",
		m_version,
		"Ciallo Engine",
		m_version,
		VK_API_VERSION_1_3
	);

	auto messengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT{
		{},
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
		debugCallback
	};

	auto createInfo = vk::InstanceCreateInfo(
		vk::InstanceCreateFlags(),
		&appInfo,
		static_cast<uint32_t>(m_validationLayers.size()),
		m_validationLayers.data(), // enabled layers
		static_cast<uint32_t>(m_instanceExtensions.size()),
		m_instanceExtensions.data() // enabled extensions
	);

	vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> c{createInfo, messengerCreateInfo};

	m_instance = vk::createInstanceUnique(c.get<vk::InstanceCreateInfo>());

	m_dldi = vk::DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);
	m_debugMessenger = m_instance->createDebugUtilsMessengerEXT(messengerCreateInfo, nullptr, m_dldi);
}

// TODO: find suitable GPU
int ciallo::vulkan::Framework::pickPhysicalDevice()
{
	return 1;
}

void ciallo::vulkan::Framework::createPhysicalDevice(const int index)
{
	auto devices = m_instance->enumeratePhysicalDevices();
	m_physicalDevice = devices[index];
}

void ciallo::vulkan::Framework::pickQueueFamily()
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

void ciallo::vulkan::Framework::createDevice()
{
	vku::DeviceMaker dm{};
	dm.queue(m_queueFamilyIndex, 1.0f);
	dm.extension(m_deviceExtensions[0]);
	m_device = dm.createUnique(m_physicalDevice);
}

void ciallo::vulkan::Framework::createCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	poolInfo.queueFamilyIndex = m_queueFamilyIndex;
	m_commandPool = m_device->createCommandPoolUnique(poolInfo);
}

bool ciallo::vulkan::Framework::isPhysicalDeviceValid(vk::PhysicalDevice device)
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

void ciallo::vulkan::Framework::addInstanceExtensions(std::vector<const char*> extensions)
{
	m_instanceExtensions.insert(m_instanceExtensions.end(), extensions.begin(), extensions.end());
}
