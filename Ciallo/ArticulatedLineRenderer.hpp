#pragma once

namespace ciallo
{
	struct ArticulatedLineStrokeData
	{
		// owned by entity
		std::vector<entt::id_type> vertexBufferIds;
		vk::UniqueDescriptorSet descriptorSet;
	};

	struct ArticulatedLineBrushData
	{
		// owned by engine, only engine knows how to construct this component.
		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::UniqueDescriptorSet descriptorSet;
	};

	enum class ArticulatedLineRenderMethodFlags
	{
		Default = 0u,
		VertexBufferColor = 1u << 0,

		_entt_enum_as_bitmask
	};
}
