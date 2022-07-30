#pragma once

#include "Buffer.hpp"
namespace ciallo::scene
{
	struct ColorCpo
	{
		glm::vec4 color;
		vulkan::Buffer buffer;
		vk::UniqueDescriptorSet descriptorSet;
	};

	struct VertexBufferCpo
	{
		vulkan::Buffer buffer;
		vk::UniqueDescriptorSet descriptorSet;
	};
}
