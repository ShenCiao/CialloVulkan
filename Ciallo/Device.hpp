#pragma once
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace ciallo::vulkan
{
	/**
	 * \brief Class for logical device, physical device, command pool and memory allocator
	 */
	class Device
	{
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;
		vk::UniqueCommandPool m_commandPool;
		uint32_t m_queueFamilyIndex = std::numeric_limits<uint32_t>::max();
		VmaAllocator m_allocator{};

	public:
		explicit Device(vk::Instance instance, int physicalDeviceIndex);
		Device(const Device& other) = delete;
		Device(Device&& other) = default;
		Device& operator=(const Device& other) = delete;
		Device& operator=(Device&& other) = default;
		~Device();
		operator vk::Device(){return *m_device;}
		
	private:
		static inline std::vector<const char*> m_extensions{
			"VK_KHR_swapchain"
		};

		void genDevice();
		void genCommandPool();
		void genAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device);

		void setPhysicalDevice(vk::PhysicalDevice device);

	public:
		static int findRequiredQueueFamily(vk::PhysicalDevice device);
		static bool isPhysicalDeviceValid(vk::PhysicalDevice device);
		static int pickPhysicalDevice(vk::Instance instance);

		vk::CommandBuffer createCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
		void executeImmediately(const std::function<void(vk::CommandBuffer)>& func);
		vk::Queue queue() const;
		vk::Device device() const {return *m_device;}
		vk::PhysicalDevice physicalDevice() const {return m_physicalDevice;}
		VmaAllocator allocator() const {return m_allocator;}
		uint32_t queueFamilyIndex() const { return m_queueFamilyIndex; }
	};
}
