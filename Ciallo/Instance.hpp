#pragma once

#include "vku.hpp"

namespace ciallo::vulkan
{
	class Instance
	{
		friend class Window;
	public:
		Instance() = default;
		Instance(const Instance& other) = delete;
		Instance(Instance&& other) = delete;
		Instance& operator=(const Instance& other) = delete;
		Instance& operator=(Instance&& other) = delete;
		~Instance();

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL
		debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		              VkDebugUtilsMessageTypeFlagsEXT messageType,
		              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		              void* pUserData);
	public:
		void create();

	public:
		vk::Instance instance() const
		{
			return *m_instance;
		}

	private:
		vk::UniqueInstance m_instance;
		vk::PhysicalDevice m_physicalDevice;
	private:
		vk::DebugUtilsMessengerEXT m_debugMessenger;
		vk::DispatchLoaderDynamic m_dldi;

		uint32_t m_version = VK_MAKE_VERSION(0, 0, 0);
		std::vector<const char*> m_validationLayers{
			"VK_LAYER_KHRONOS_validation"
		};
		std::vector<const char*> m_instanceExtensions{
			"VK_EXT_debug_utils"
		};
	public:
		void addInstanceExtensions(std::vector<const char*> extensions);
	};
}
