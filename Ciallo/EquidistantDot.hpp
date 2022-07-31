#pragma once

#include "Device.hpp"
#include "Image.hpp"
#include "ShaderModule.hpp"
#include "Renderer.hpp"

namespace ciallo::rendering
{
	class EquidistantDotEngine
	{
		vk::Device m_device;
		vulkan::ShaderModule m_compShader;
		vulkan::ShaderModule m_vertShader;
		vulkan::ShaderModule m_fragShader;
		vulkan::ShaderModule m_geomShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_pipeline;

		vulkan::Buffer m_indirectDrawBuffer;
		vulkan::Buffer m_dotBuffer; // buffer for rendered quad
		vulkan::Buffer m_inputBuffer; // buffer for data points input

		vk::UniquePipelineLayout m_compPipelineLayout;
		vk::UniquePipeline m_compPipeline;
		vk::UniqueDescriptorSetLayout m_compDescriptorSetLayout;
		vk::DescriptorSet m_compDescriptorSet;
	public:
		explicit EquidistantDotEngine(vulkan::Device* device);

		void genPipelineDynamic();

		void genCompDescriptorSet(vk::DescriptorPool pool);
		void genCompPipeline();
		void renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target);
		void genInputBuffer(VmaAllocator allocator);
		void genAuxiliaryBuffer(VmaAllocator allocator);
		void compute(vk::CommandBuffer cb);
	};

	class EquidistantDotRenderer: public Renderer
	{
		vk::Pipeline m_pipeline;
		vk::PipelineLayout m_pipelineLayout;
		vk::Pipeline m_computePipeline;
		vk::PipelineLayout m_computePipelineLayout;

		vulkan::Buffer m_indirectDrawBuffer;
		vulkan::Buffer m_dotBuffer; // buffer for graphics pipeline input.
	public:
		EquidistantDotRenderer() = default;
		EquidistantDotRenderer(const EquidistantDotRenderer& other) = default;
		EquidistantDotRenderer(EquidistantDotRenderer&& other) noexcept = default;
		EquidistantDotRenderer& operator=(const EquidistantDotRenderer& other) = default;
		EquidistantDotRenderer& operator=(EquidistantDotRenderer&& other) noexcept = default;
		~EquidistantDotRenderer() override = default;

		void render(vk::CommandBuffer cb, entt::handle object) override;
	};
}
