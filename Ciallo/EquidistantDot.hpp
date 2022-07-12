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

		vulkan::Buffer m_dotBuffer;// buffer for rendered quad
		vulkan::Buffer m_inputBuffer;// buffer for data points input

		vulkan::ShaderModule m_computeShader;
		vk::UniquePipelineLayout m_computePipelineLayout;
		vk::UniquePipeline m_computePipeline;
		vk::UniqueDescriptorSetLayout m_computeDescriptorSetLayout;
		vk::DescriptorSet m_computeDescriptorSet;
	public:
		explicit EquidistantDot(vulkan::Device* device);

		void genPipelineLayout();

		vk::UniquePipeline m_pipeline;
		void genPipelineDynamic();

		void genComputeDescriptorSet(vk::DescriptorPool pool);
		void genComputePipeline();

		void renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target);
		void genInputBuffer(VmaAllocator allocator);
		void genVertexBuffer(VmaAllocator allocator);
		void compute(vk::CommandBuffer cb);
	};
}
