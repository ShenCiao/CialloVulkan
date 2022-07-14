#pragma once

#include "Device.hpp"
#include "Image.hpp"
#include "ShaderModule.hpp"

namespace ciallo::rendering
{
	class EquidistantDot
	{
		vk::Device m_device;
		vulkan::ShaderModule m_vertShader;
		vulkan::ShaderModule m_fragShader;
		vulkan::ShaderModule m_geomShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vulkan::Image* m_target = nullptr;

		vulkan::Buffer m_indirectDrawBuffer;
		vulkan::Buffer m_dotBuffer;// buffer for rendered quad
		vulkan::Buffer m_inputBuffer;// buffer for data points input

		vulkan::ShaderModule m_compShader;
		vk::UniquePipelineLayout m_compPipelineLayout;
		vk::UniquePipeline m_compPipeline;
		vk::UniqueDescriptorSetLayout m_compDescriptorSetLayout;
		vk::DescriptorSet m_compDescriptorSet;
	public:
		explicit EquidistantDot(vulkan::Device* device);

		vk::UniquePipeline m_pipeline;
		void genPipelineDynamic();

		void genCompDescriptorSet(vk::DescriptorPool pool);
		void genCompPipeline();

		void renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target);
		void genInputBuffer(VmaAllocator allocator);
		void genAuxiliaryBuffer(VmaAllocator allocator);
		void compute(vk::CommandBuffer cb);
	};
}
