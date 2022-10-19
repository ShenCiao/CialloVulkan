#pragma once

#include <vulkan/vulkan.hpp>

namespace ciallo::vulkan
{
	class Instance
	{
		friend class Window;
		friend class Device;
		vk::UniqueInstance m_instance;
		vk::DebugUtilsMessengerEXT m_debugMessenger;
		vk::DispatchLoaderDynamic m_dldi;

		uint32_t m_version = VK_MAKE_VERSION(0, 0, 0);
		static inline std::vector<const char*> m_validationLayers{
			"VK_LAYER_KHRONOS_validation",
		};
		static inline std::vector<const char*> m_instanceExtensions{
			"VK_EXT_debug_utils"
		};
		static inline std::vector<const char*> m_deviceExtensions{
			"VK_KHR_swapchain",
			//ShenCiao's AMD Gpu(integrated) does not support these :(. I need them!!!
			// "VK_EXT_blend_operation_advanced",
			// "VK_EXT_vertex_input_dynamic_state",
		};

	public:
		Instance();
		Instance(const Instance& other) = delete;
		Instance(Instance&& other) = default;
		Instance& operator=(const Instance& other) = delete;
		Instance& operator=(Instance&& other) = default;
		~Instance();
		operator vk::Instance() const;

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL
		debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		              VkDebugUtilsMessageTypeFlagsEXT messageType,
		              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		              void* pUserData);

		void genInstance();

	public:
		vk::Instance instance() const
		{
			return *m_instance;
		}

		static void addExtensions(const std::vector<const char *>& extensions);
		static int findRequiredQueueFamily(vk::PhysicalDevice device, vk::SurfaceKHR surface);
		static bool isPhysicalDeviceValid(vk::PhysicalDevice device, vk::SurfaceKHR surface);
		static vk::PhysicalDevice pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);

	};
}
