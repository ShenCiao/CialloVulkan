#pragma once

#include "Device.hpp"
#include "Image.hpp"
#include "ShaderModule.hpp"

namespace ciallo::rendering
{
	class EquidistantDot
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
		explicit EquidistantDot(vulkan::Device* device);

		void genPipelineDynamic();

		void genCompDescriptorSet(vk::DescriptorPool pool);
		void genCompPipeline();
		void renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target);
		void genInputBuffer(VmaAllocator allocator);
		void genAuxiliaryBuffer(VmaAllocator allocator);
		void compute(vk::CommandBuffer cb);

		void render(vk::CommandBuffer cb, const std::vector<vk::DescriptorSet>& vertInputs,
		            const std::vector<vk::DescriptorSet>& descriptors)
		{
			// Compute
			cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_compPipelineLayout, 0, vertInputs, nullptr);
			cb.bindPipeline(vk::PipelineBindPoint::eCompute, *m_compPipeline);
			cb.dispatch(1, 1, 1);

			// Sync
			const vk::MemoryBarrier2 drawIndirectBarrier{
				vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderStorageWrite,
				vk::PipelineStageFlagBits2::eDrawIndirect, vk::AccessFlagBits2::eIndirectCommandRead
			};
			const vk::MemoryBarrier2 vertexBarrier{
				vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderStorageWrite,
				vk::PipelineStageFlagBits2::eVertexInput, vk::AccessFlagBits2::eVertexAttributeRead
			};
			const std::vector barriers = {drawIndirectBarrier, vertexBarrier};
			cb.pipelineBarrier2({{}, barriers, {}, {}});

			// Graphics
			std::vector<vk::Buffer> vertexBuffers{m_dotBuffer};
			cb.bindVertexBuffers(0, vertexBuffers, {0});
			cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
			cb.drawIndirect(m_indirectDrawBuffer, 0, 1, {});
		}
	};
}
