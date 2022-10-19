#include "pch.hpp"
#define VMA_IMPLEMENTATION
#include "Device.hpp"
#include "Instance.hpp"

namespace ciallo::vulkan
{
	Device::Device(vk::Instance instance, vk::PhysicalDevice physicalDevice, uint32_t queueFamilyIndex):
		m_physicalDevice(physicalDevice), m_queueFamilyIndex(queueFamilyIndex)
	{
		genDevice();
		genCommandPool();
		genDescriptorPool();
		genAllocator(instance, physicalDevice, *m_device);
	}

	Device::~Device()
	{
		vmaDestroyAllocator(m_allocator);
	}

	void Device::genDevice()
	{
		std::vector<float> priorities{1.0f};
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{{}, m_queueFamilyIndex, priorities};

		vk::DeviceCreateInfo deviceCreateInfo{
			{},
			deviceQueueCreateInfo,
			{},
			Instance::m_deviceExtensions,
		};

		// Warning: may encounter features do not supported
		vk::PhysicalDeviceFeatures physicalDeviceFeatures{};
		physicalDeviceFeatures.setGeometryShader(VK_TRUE)
		                      .setTessellationShader(VK_TRUE)
		                      .setWideLines(VK_TRUE)
		                      .setShaderClipDistance(VK_TRUE);
		vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2{physicalDeviceFeatures};

		vk::PhysicalDeviceVulkan12Features vulkan12Features{};
		vulkan12Features.setUniformBufferStandardLayout(VK_TRUE);

		vk::PhysicalDeviceVulkan13Features vulkan13Features{};
		vulkan13Features.setDynamicRendering(VK_TRUE)
		                .setSynchronization2(VK_TRUE);

		vk::StructureChain c(deviceCreateInfo, physicalDeviceFeatures2, vulkan13Features);
		m_device = m_physicalDevice.createDeviceUnique(c.get<vk::DeviceCreateInfo>());
	}

	vk::CommandBuffer Device::createCommandBuffer(vk::CommandBufferLevel level)
	{
		vk::CommandBufferAllocateInfo info{
			*m_commandPool,
			level,
			1u
		};
		return m_device->allocateCommandBuffers(info)[0];
	}

	vk::UniqueDescriptorSet Device::createDescriptorSetUnique(vk::DescriptorSetLayout layout) const
	{
		vk::DescriptorSetAllocateInfo info{*m_descriptorPool, layout};
		return std::move(m_device->allocateDescriptorSetsUnique(info)[0]);
	}

	void Device::genCommandPool()
	{
		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = m_queueFamilyIndex;
		m_commandPool = m_device->createCommandPoolUnique(poolInfo);
	}

	void Device::genDescriptorPool()
	{
		vk::DescriptorPoolCreateInfo poolInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			MAX_SIZE * static_cast<uint32_t>(m_descriptorPoolSizes.size()),
			m_descriptorPoolSizes
		);
		m_descriptorPool = m_device->createDescriptorPoolUnique(poolInfo);
	}

	void Device::genAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device)
	{
		VmaAllocatorCreateInfo info{};
		info.physicalDevice = physicalDevice;
		info.device = device;
		info.instance = instance;

		vmaCreateAllocator(&info, &m_allocator);
	}

	void Device::executeImmediately(const std::function<void(vk::CommandBuffer)>& func)
	{
		vk::CommandBufferAllocateInfo info{
			*m_commandPool,
			vk::CommandBufferLevel::ePrimary,
			1
		};
		auto cb = std::move(m_device->allocateCommandBuffersUnique(info)[0]);
		cb->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
		func(*cb);
		cb->end();

		vk::UniqueFence fence = m_device->createFenceUnique({});
		vk::SubmitInfo si{};
		si.setCommandBuffers(*cb);
		queue().submit(si, *fence);
		[[maybe_unused]] auto result = m_device->waitForFences(*fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	}

	vk::Queue Device::queue() const
	{
		return m_device->getQueue(m_queueFamilyIndex, 0);
	}
}
