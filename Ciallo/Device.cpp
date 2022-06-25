#include "pch.hpp"
#define VMA_IMPLEMENTATION
#include "Device.hpp"

namespace ciallo::vulkan
{
	Device::Device(vk::Instance instance, int physicalDeviceIndex)
	{
		auto physicalDevice = instance.enumeratePhysicalDevices()[physicalDeviceIndex];
		setPhysicalDevice(physicalDevice);
		int index = findRequiredQueueFamily(physicalDevice);
		if (index < 0)
		{
			throw std::runtime_error("No required index, check for physical device validity before using it!");
		}
		m_queueFamilyIndex = index;
		genDevice();
		genCommandPool();
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
		vk::PhysicalDeviceFeatures physicalDeviceFeatures;
		physicalDeviceFeatures.setGeometryShader(VK_TRUE);
		physicalDeviceFeatures.setTessellationShader(VK_TRUE);
		deviceCreateInfo.setPEnabledFeatures(&physicalDeviceFeatures);

		vk::PhysicalDeviceMultiviewFeatures physicalDeviceMultiviewFeatures;
		physicalDeviceMultiviewFeatures.setMultiview(VK_TRUE);
		deviceCreateInfo.pNext = &physicalDeviceMultiviewFeatures;

		m_device = m_physicalDevice.createDeviceUnique(deviceCreateInfo);
	}

	void Device::setPhysicalDevice(vk::PhysicalDevice device)
	{
		m_physicalDevice = device;
	}

	int Device::findRequiredQueueFamily(vk::PhysicalDevice device)
	{
		auto queueFamilies = device.getQueueFamilyProperties();
		// need a queue family be able to graphics, compute, transfer and presents(unchecked)
		vk::QueueFlags req = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;
		for (const auto [i, queueFamily] : views::enumerate(queueFamilies))
		{
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & req) == req)
			{
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	bool Device::isPhysicalDeviceValid(vk::PhysicalDevice device)
	{
		auto queueFamilies = device.getQueueFamilyProperties();
		bool queueFamilyFound = false;
		for (const auto [i, queueFamily] : views::enumerate(queueFamilies))
		{
			if (queueFamily.queueCount > 0 &&
				queueFamily.queueFlags & vk::QueueFlagBits::eGraphics &&
				queueFamily.queueFlags & vk::QueueFlagBits::eCompute &&
				queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
			{
				queueFamilyFound = true;
				break;
			}
		}

		// device extension support
		auto extensions = device.enumerateDeviceExtensionProperties();
		std::unordered_set<std::string> requiredExtensions{m_extensions.begin(), m_extensions.end()};

		for (const auto& extension : extensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}
		bool extensionsFound = requiredExtensions.empty();


		// may need check swapchain
		return queueFamilyFound && extensionsFound;
	}

	/**
	 * \brief Choose from available physical device
	 * \param instance vulkan instance
	 * \return physical device index
	 */
	int Device::pickPhysicalDevice(vk::Instance instance)
	{
		//TODO: make it portable
		return 1;
		auto physicalDevices = instance.enumeratePhysicalDevices();
		for (auto device : physicalDevices)
		{
			isPhysicalDeviceValid(device);
		}
	}

	void Device::genCommandPool()
	{
		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = m_queueFamilyIndex;
		m_commandPool = m_device->createCommandPoolUnique(poolInfo);
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
