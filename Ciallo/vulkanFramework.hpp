#pragma once

#include "vku.hpp"

namespace ciallo::vulkan
{
	class Framework
	{
		friend class Window;
	public:
		Framework() = default;
		Framework(const Framework& other) = delete;
		Framework(Framework&& other) = delete;
		Framework& operator=(const Framework& other) = delete;
		Framework& operator=(Framework&& other) = delete;
		~Framework();

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL
		debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		              VkDebugUtilsMessageTypeFlagsEXT messageType,
		              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		              void* pUserData);
	public:
		void createInstance();
		int pickPhysicalDevice(); // return index of GPU
		void createPhysicalDevice(int index);
		void pickQueueFamily();
		void createDevice();
		void createCommandPool();
		bool isPhysicalDeviceValid(vk::PhysicalDevice device);
	public:
		vk::Instance instance() const
		{
			return *m_instance;
		}


	private:
		vk::UniqueInstance m_instance;
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;
		vk::UniqueCommandPool m_commandPool;

	private:
		// Warning: queue be able to graphics compute and (assumed) presents, may cause problem
		uint32_t m_queueFamilyIndex;
		vk::DispatchLoaderDynamic m_dldi; // 
		vk::DebugUtilsMessengerEXT m_debugMessenger;

		uint32_t m_version = VK_MAKE_API_VERSION(0, 0, 0, 0);
		std::vector<const char*> m_validationLayers{
			"VK_LAYER_KHRONOS_validation"
		};
		std::vector<const char*> m_instanceExtensions{
			"VK_EXT_debug_utils"
		};
		std::vector<const char*> m_deviceExtensions{
			"VK_KHR_swapchain"
		}; // Unused member
	public:
		void addInstanceExtensions(std::vector<const char*> extensions);
	};
}
