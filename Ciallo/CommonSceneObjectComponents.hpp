#pragma once

#include "Buffer.hpp"
#include "Renderer.hpp"

namespace ciallo::scene
{
	struct ColorCpo
	{
		glm::vec4 color;
		vulkan::Buffer buffer;
	};

	struct VertexBufferCpo
	{
		std::vector<vulkan::Buffer> buffers;
		uint32_t vertexCount = 0u;
	};

	struct VertexBufferDesSCpo
	{
		// A descriptorSet points to vertex buffer for compute (may not needed).
		vk::UniqueDescriptorSet vertexBufferDesS; 
	};

	
	struct PipelineResourceCpo
	{
		// A descriptorSet of Images, Samplers, buffers that need to pop into graphics pipeline.
		vk::UniqueDescriptorSet pipelineDesS;
	};

	struct RendererCpo
	{
		rendering::Renderer* renderer;
	};
}
