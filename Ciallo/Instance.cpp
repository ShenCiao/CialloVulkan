#include "pch.hpp"

#include "Instance.hpp"

namespace ciallo::vulkan
{
	Instance::Instance()
	{
		genInstance();
	}

	Instance::~Instance()
	{
		m_instance->destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_dldi);
	}

	Instance::operator vk::Instance() const
	{
		return *m_instance;
	}

	VkBool32 Instance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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

		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT && 
			messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		{
			spdlog::info("{}", pCallbackData->pMessage);
		}
		return VK_FALSE;
	}

	void Instance::genInstance()
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

		std::vector<vk::ValidationFeatureEnableEXT> enableValidateFeatures{
			// vk::ValidationFeatureEnableEXT::eGpuAssisted,
			// vk::ValidationFeatureEnableEXT::eBestPractices,
			vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
			vk::ValidationFeatureEnableEXT::eDebugPrintf,
		};
		vk::ValidationFeaturesEXT validationFeatures{enableValidateFeatures};

		auto createInfo = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			m_validationLayers, // enabled layers
			m_instanceExtensions // enabled extensions
		);

		vk::StructureChain c{createInfo, validationFeatures, messengerCreateInfo};

		m_instance = vk::createInstanceUnique(c.get<vk::InstanceCreateInfo>());

		m_dldi = vk::DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);
		m_debugMessenger = m_instance->createDebugUtilsMessengerEXT(messengerCreateInfo, nullptr, m_dldi);
	}

	void Instance::addExtensions(const std::vector<const char*>& extensions)
	{
		m_instanceExtensions.insert(m_instanceExtensions.end(), extensions.begin(), extensions.end());
	}

		// need a queue family be able to graphics, compute, transfer and presents
	int Instance::findRequiredQueueFamily(vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
		auto queueFamilies = device.getQueueFamilyProperties();

		vk::QueueFlags req = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;
		for (const auto&& [i, qF] : views::enumerate(queueFamilies))
		{
			if (qF.queueCount > 0 && (qF.queueFlags & req) == req)
			{
				if (bool support = device.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
					return static_cast<int>(i);
			}
		}
		return -1;
	}

	bool Instance::isPhysicalDeviceValid(vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
		using vk::QueueFlags;
		using vk::QueueFlagBits;
		// find queue family
		bool queueFamilyFound = false;
		if (int i = findRequiredQueueFamily(device, surface); i != -1)
		{
			queueFamilyFound = true;
		}

		// device extension support
		auto extensions = device.enumerateDeviceExtensionProperties();
		std::unordered_set<std::string> requiredExtensions{m_deviceExtensions.begin(), m_deviceExtensions.end()};

		for (const auto& extension : extensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}
		bool extensionsFound = requiredExtensions.empty();

		// Do no need to check the swapchain
		return queueFamilyFound && extensionsFound;
	}

	/**
	 * \brief Choose from available physical device
	 * \param instance Vulkan instance
	 * \param surface Vulkan surface, use it for checking present capability of queue 
	 * \return Physical device index
	 */
	vk::PhysicalDevice Instance::pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface)
	{
		auto physicalDevices = instance.enumeratePhysicalDevices();
		vk::PhysicalDevice nearest;
		for (auto device : physicalDevices)
		{
			if (isPhysicalDeviceValid(device, surface))
			{
				nearest = device;
				auto prop = device.getProperties2().properties;
				if (prop.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
				{
					return device;
				}
			}
		}

		if(nearest)
		{
			return nearest;
		}

		throw std::runtime_error("Fail to find appropriate physical Device!");
	}
}
