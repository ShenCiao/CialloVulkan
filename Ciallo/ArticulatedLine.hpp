#pragma once

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
	class ArticulatedLineEngineTemp
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
	public:
		std::vector<Vertex> vertices; // delete it after...
		explicit ArticulatedLineEngineTemp(vulkan::Device* device);
		void genPipelineLayout();
		void genPipelineDynamic();
		void renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target);
		void genVertexBuffer(VmaAllocator allocator);
	public:
		void assignRenderer(entt::registry& r, entt::entity e, vulkan::Device* device);

		static inline std::array<entt::observer, 1> obs;
		static void connect(entt::registry& r);
		void update(entt::registry& r);
	};

	struct ArticulatedLineEngine
	{
		// more appropriate data structure needed
		vk::UniqueShaderModule vertShader;
		vk::UniqueShaderModule geomShader;
		vk::UniqueShaderModule fragShader;
		vk::UniqueDescriptorSetLayout strokeDescriptorSetLayout;
		vk::UniqueDescriptorSetLayout brushDescriptorSetLayout;
		vk::UniquePipelineLayout pipelineLayout;
		vk::UniquePipeline pipeline;

		explicit ArticulatedLineEngine(vk::Device device);
		void init(vk::Device device);

		void assignBrushRenderingData(entt::registry& r, entt::entity brushE, const ArticulatedLineSettings& settings);
		void assignStrokeRenderingData(entt::registry& r, entt::entity strokeE, const ArticulatedLineSettings& settings);
		void removeRenderingData(entt::registry& r, entt::entity e);
		void render(entt::registry& r, entt::entity brushE, entt::entity strokeE, vk::CommandBuffer cb);
	};
}
