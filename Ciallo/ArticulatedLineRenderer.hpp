#pragma once

namespace ciallo
{
	struct ArticulatedLineRenderingData
	{
		// owned by engine, only engine knows how to construct this object.
		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::DescriptorSetLayout descriptorSetLayout;

		// owned by entity
		vk::UniqueDescriptorSet entityDescriptorSet;
		uint32_t vertexCount = 0;

		static inline entt::observer ob{};
		static void connect(entt::registry& r);
		static void update(entt::registry& r);
	};

	struct ArticulatedLineDefaultRenderTag
	{
		static void render(entt::registry& r, entt::entity e, vk::CommandBuffer cb);
	};
}
