#pragma once

#include "Buffer.hpp"
#include "Image.hpp"

namespace ciallo
{
	struct ArticulatedLineRenderObjectCpo
	{
		std::vector<vulkan::Buffer> vertexBuffers;
		// A descriptorSet at per object level.
		vk::UniqueDescriptorSet objectDescriptorSet;
		vulkan::Buffer colorBuffer;
	};

	class ArticulatedLineRenderObjectAssigner
	{
		static void update(entt::registry& registry);
	};
}
