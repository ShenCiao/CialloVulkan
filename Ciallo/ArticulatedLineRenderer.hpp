#pragma once

#include "Buffer.hpp"
#include "EntityRenderer.hpp"

namespace ciallo
{
	struct ArticulatedLineRenderer : EntityRenderer
	{
		// owned by engine
		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::DescriptorSetLayout descriptorSetLayout;

		// owned by renderer
		uint32_t vertexCount = 0;
		std::vector<vulkan::Buffer> vertexBuffers{};
		vk::UniqueDescriptorSet objectDescriptorSet; // A descriptorSet at per object level.
		vulkan::Buffer colorBuffer;
	
		ArticulatedLineRenderer() = default;
		ArticulatedLineRenderer(const ArticulatedLineRenderer& other) = delete;
		ArticulatedLineRenderer(ArticulatedLineRenderer&& other) = default;
		ArticulatedLineRenderer& operator=(const ArticulatedLineRenderer& other) = delete;
		ArticulatedLineRenderer& operator=(ArticulatedLineRenderer&& other) = default;
		~ArticulatedLineRenderer() override = default;

		void render(vk::CommandBuffer cb) override;

		static inline std::array<entt::observer, 3> obs;
		static void update(entt::registry& r);
		static void construct(entt::registry& r, entt::entity e);
		static void connect(entt::registry& r);
		static void disconnect(entt::registry& r);
	};
}
