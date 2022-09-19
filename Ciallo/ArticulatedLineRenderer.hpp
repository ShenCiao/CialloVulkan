#pragma once

#include "Buffer.hpp"
#include "EntityRenderer.hpp"

namespace ciallo
{
	struct ArticulatedLineRenderingData
	{
		// owned by engine
		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::DescriptorSetLayout descriptorSetLayout;

		// owned by entity
		vk::UniqueDescriptorSet objectDescriptorSet;
		uint32_t vertexCount = 0;

		static inline entt::observer ob{};
		static void connect(entt::registry& r);
		static void update(entt::registry& r);
	};

	struct ArticulatedLineDefaultRenderer
	{
		static void render(entt::registry& r, entt::entity e, vk::CommandBuffer cb);
	};
}
