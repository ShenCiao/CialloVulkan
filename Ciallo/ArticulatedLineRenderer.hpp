#pragma once

#include "Buffer.hpp"
#include "Device.hpp"
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

		static void update(entt::registry& r);
	};
}
