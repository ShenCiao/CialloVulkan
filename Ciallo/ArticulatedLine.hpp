#pragma once

#include "ArticulatedLineRenderer.hpp"
#include "Device.hpp"
#include "Image.hpp"
#include "ShaderModule.hpp"

namespace ciallo
{
	struct ArticulatedLineSettings
	{
		bool enableFalloff = false;
	};

	/**
	 * \brief Need a factory pattern here. Engine is the factory of renderer.
	 */
	class ArticulatedLineEngine
	{
		struct Vertex
		{
			glm::vec2 pos;
			glm::vec4 color;
			float width;
		};

		vk::Device m_device;
		vulkan::ShaderModule m_vertShader;
		vulkan::ShaderModule m_fragShader;
		vulkan::ShaderModule m_geomShader;
		vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_pipeline;
		vulkan::Buffer m_vertBuffer;
		std::vector<ArticulatedLineRenderer> m_renderers;
	public:
		std::vector<Vertex> vertices; // delete it after...
		explicit ArticulatedLineEngine(vulkan::Device* device);
		void genPipelineLayout();
		void genPipelineDynamic();
		void renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target);
		void genVertexBuffer(VmaAllocator allocator);
	public:
		void assignRenderer(entt::registry& r, entt::entity e);

		static inline std::array<entt::observer, 1> obs;
		static void connect(entt::registry& r);
		void update(entt::registry& r);
	};
}
