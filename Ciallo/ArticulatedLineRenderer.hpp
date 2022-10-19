#pragma once
#include "Buffer.hpp"

namespace ciallo
{
	struct ArticulatedLineStrokeCpo
	{
		// owned by entity
		std::vector<vulkan::Buffer> vertexBuffers;
	};

	struct ArticulatedLineBrushCpo
	{
		// owned by engine, only engine knows how to construct this component.
		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::UniqueDescriptorSet descriptorSet;
	};
}
