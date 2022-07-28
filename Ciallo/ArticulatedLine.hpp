#pragma once

#include "Device.hpp"
#include "Image.hpp"
#include "ShaderModule.hpp"

namespace ciallo::rendering
{
	class ArticulatedLine
	{
		vk::Device m_device;
		vulkan::ShaderModule m_vertShader;
		vulkan::ShaderModule m_fragShader;
		vulkan::ShaderModule m_geomShader;
		vk::UniquePipeline m_pipeline;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniqueRenderPass m_renderPass;
		vk::UniqueFramebuffer m_framebuffer;
		vulkan::Buffer m_vertBuffer;
	public:
		explicit ArticulatedLine(vulkan::Device* device);

		void genPipelineLayout();

		void genPipelineDynamic();

		void renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target);
		void genVertexBuffer(VmaAllocator allocator);
	};
}

namespace ciallo::brush
{
	struct ArticulatedLineSettingsCpo
	{
		
	};
}