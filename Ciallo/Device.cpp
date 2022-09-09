#include "pch.hpp"
#define VMA_IMPLEMENTATION
#include "Device.hpp"

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
			m_extensions,
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

	// need a queue family be able to graphics, compute, transfer and presents
	int Device::findRequiredQueueFamily(vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
		auto queueFamilies = device.getQueueFamilyProperties();

		vk::QueueFlags req = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;
		for (const auto [i, qF] : views::enumerate(queueFamilies))
		{
			if (qF.queueCount > 0 && (qF.queueFlags & req) == req)
			{
				bool support = device.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface);
				if (support)
					return static_cast<int>(i);
			}
		}
		return -1;
	}

	bool Device::isPhysicalDeviceValid(vk::PhysicalDevice device, vk::SurfaceKHR surface)
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
		std::unordered_set<std::string> requiredExtensions{m_extensions.begin(), m_extensions.end()};

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
	vk::PhysicalDevice Device::pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface)
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
