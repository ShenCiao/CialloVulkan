#pragma once
#include <functional>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace ciallo::vulkan
{
	/**
	 * \brief Class for logical device, physical device, command pool, descriptor pool and memory allocator.
	 * Can be implicitly converted to the allocator since allocator is nothing more than the device and instance.
	 */
	class Device
	{
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;
		vk::UniqueCommandPool m_commandPool;
		vk::UniqueDescriptorPool m_descriptorPool;
		uint32_t m_queueFamilyIndex = std::numeric_limits<uint32_t>::max();
		VmaAllocator m_allocator{};

		constexpr static int MAX_SIZE = 128;
		std::vector<vk::DescriptorPoolSize> m_descriptorPoolSizes{
			{vk::DescriptorType::eSampler, MAX_SIZE},
			{vk::DescriptorType::eCombinedImageSampler, MAX_SIZE},
			{vk::DescriptorType::eSampledImage, MAX_SIZE},
			{vk::DescriptorType::eStorageImage, MAX_SIZE},
			{vk::DescriptorType::eUniformTexelBuffer, MAX_SIZE},
			{vk::DescriptorType::eStorageTexelBuffer, MAX_SIZE},
			{vk::DescriptorType::eUniformBuffer, MAX_SIZE},
			{vk::DescriptorType::eStorageBuffer, MAX_SIZE},
			{vk::DescriptorType::eUniformBufferDynamic, MAX_SIZE},
			{vk::DescriptorType::eStorageBufferDynamic, MAX_SIZE},
			{vk::DescriptorType::eInputAttachment, MAX_SIZE},
		};

	public:
		explicit Device(vk::Instance instance, vk::PhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
		Device(const Device& other) = delete;
		Device(Device&& other) = default;
		Device& operator=(const Device& other) = delete;
		Device& operator=(Device&& other) = default;
		~Device();
		operator vk::Device() { return *m_device; }
		operator VmaAllocator() const { return m_allocator; }

	private:
		static inline std::vector<const char*> m_extensions{
			"VK_KHR_swapchain",
			//ShenCiao's AMD Gpu(integrated) does not support these :(. I need them!!!
			// "VK_EXT_blend_operation_advanced",
			// "VK_EXT_vertex_input_dynamic_state",
		};

		void genDevice();
		void genCommandPool();
		void genDescriptorPool();
		void genAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device);
	public:
		static int findRequiredQueueFamily(vk::PhysicalDevice device, vk::SurfaceKHR surface);
		static bool isPhysicalDeviceValid(vk::PhysicalDevice device, vk::SurfaceKHR surface);
		static vk::PhysicalDevice pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);

		void executeImmediately(const std::function<void(vk::CommandBuffer)>& func);
		vk::CommandBuffer createCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
		vk::Queue queue() const;
		vk::Device device() const { return *m_device; }
		vk::PhysicalDevice physicalDevice() const { return m_physicalDevice; }
		VmaAllocator allocator() const { return m_allocator; }
		uint32_t queueFamilyIndex() const { return m_queueFamilyIndex; }
		vk::DescriptorPool descriptorPool() const { return *m_descriptorPool; }
	};
}
