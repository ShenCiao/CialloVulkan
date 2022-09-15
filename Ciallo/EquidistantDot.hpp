#pragma once

#include "Device.hpp"
#include "Image.hpp"
#include "ShaderModule.hpp"
#include "EntityRenderer.hpp"

namespace ciallo
{
	class EquidistantDotEngine
	{
		struct Vertex
		{
			glm::vec2 pos;
			float width;
			float _pad0;
			glm::vec4 color;
		};

		vk::Device m_device;
		vulkan::ShaderModule m_compShader;
		vulkan::ShaderModule m_vertShader;
		vulkan::ShaderModule m_fragShader;
		vulkan::ShaderModule m_geomShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_pipeline;

		vulkan::Buffer m_indirectDrawBuffer;
		vulkan::Buffer m_vertBuffer; // a large buffer for compute shader to output
		vulkan::Buffer m_inputBuffer; // buffer for data points input
		vulkan::Buffer m_tempBufferForSpacing;

		vk::UniquePipelineLayout m_compPipelineLayout;
		vk::UniquePipeline m_compPipeline;
		vk::UniqueDescriptorSetLayout m_compDescriptorSetLayout;
		vk::DescriptorSet m_compDescriptorSet;
	public:
		std::vector<Vertex> vertices; // delete it after...
		float spacing = 0.02f;
		explicit EquidistantDotEngine(vulkan::Device* device);

		void genPipelineDynamic();

		void genCompDescriptorSet(vk::DescriptorPool pool);
		void genCompPipeline();
		void renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target);
		void genInputBuffer(VmaAllocator allocator);
		void genAuxiliaryBuffer(VmaAllocator allocator);
		void compute(vk::CommandBuffer cb);
	};

	class EquidistantDotRenderer : public EntityRenderer
	{
		vk::Pipeline m_pipeline;
		vk::PipelineLayout m_pipelineLayout;
		vk::Pipeline m_computePipeline;
		vk::PipelineLayout m_computePipelineLayout;

		vulkan::Buffer m_indirectDrawBuffer;
		vulkan::Buffer m_dotBuffer; // buffer for graphics pipeline input.
	public:
		EquidistantDotRenderer() = default;
		EquidistantDotRenderer(const EquidistantDotRenderer& other) = delete;
		EquidistantDotRenderer(EquidistantDotRenderer&& other) noexcept = default;
		EquidistantDotRenderer& operator=(const EquidistantDotRenderer& other) = delete;
		EquidistantDotRenderer& operator=(EquidistantDotRenderer&& other) noexcept = default;
		~EquidistantDotRenderer() override = default;

		void render(vk::CommandBuffer cb) override;
	};
}
