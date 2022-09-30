#pragma once

namespace ciallo
{
	struct ArticulatedLineStrokeCpo
	{
		// owned by entity
		std::vector<entt::id_type> vertexBufferIds;
		vk::UniqueDescriptorSet descriptorSet;
	};

	struct ArticulatedLineBrushCpo
	{
		// owned by engine, only engine knows how to construct this component.
		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::UniqueDescriptorSet descriptorSet;
	};
}
