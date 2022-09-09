#pragma once

#include "Buffer.hpp"
#include "ObjectRenderer.hpp"

namespace ciallo::temp
{
	struct ColorCpo
	{
		glm::vec4 color;
		vulkan::Buffer buffer;
	};

	struct VertexBufferCpo
	{
		// Each attributes has its own buffer.
		std::vector<vulkan::Buffer> buffers;
		uint32_t vertexCount = 0u;
		vk::UniqueDescriptorSet descriptorSet;
	};

	
	struct PipelineDescriptorSetCpo
	{
		// A descriptorSet of Images, Samplers, buffers that need to pop into graphics pipeline at per object level.
		vk::UniqueDescriptorSet pipelineDesS;
	};

	struct RendererCpo
	{
		ObjectRenderer* renderer;
	};
}
