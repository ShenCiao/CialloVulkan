#include "pch.hpp"

#include "Instance.hpp"

namespace ciallo::vulkan
{
	Instance::Instance()
	{
		// auto availableLayerProperties = vk::enumerateInstanceLayerProperties();
		// for (auto layer : availableLayerProperties)
		// {
		// 	spdlog::info("layer name: {}", layer.layerName);
		// }
		genInstance();

		// //TODO: Delete this after ...
		// auto devices = m_instance->enumeratePhysicalDevices();
		// for (auto const& d : devices)
		// {
		// 	auto properties = d.getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceSubgroupProperties,
		// 	                                   vk::PhysicalDeviceSubgroupSizeControlProperties>();
		// 	auto physicalDeviceProperties = properties.get<vk::PhysicalDeviceProperties2>().properties;
		// 	auto subgroupProperties = properties.get<vk::PhysicalDeviceSubgroupProperties>();
		// 	auto& physicalDeviceSubgroupSizeControlProperties = properties.get<
		// 		vk::PhysicalDeviceSubgroupSizeControlProperties>();
		// }
	}

	Instance::~Instance()
	{
		m_instance->destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_dldi);
	}

	Instance::operator vk::Instance()
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
}
