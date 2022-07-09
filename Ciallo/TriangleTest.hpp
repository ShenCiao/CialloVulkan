#pragma once

#include "Device.hpp"
#include "Image.hpp"
#include "ShaderModule.hpp"
#include "vku.hpp"

namespace ciallo::rendering
{
	class TriangleTest
	{
		vk::Device m_device;
		vulkan::ShaderModule m_vert;
		vulkan::ShaderModule m_frag;
		vk::UniquePipeline m_pipeline;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniqueRenderPass m_renderPass;
		vk::UniqueFramebuffer m_framebuffer;
		vulkan::Image* m_target;
	public:
		TriangleTest(vulkan::Device* device, vulkan::Image* target);

		void genPipelineLayout();

		void genPipelineDynamic();

		void genRenderPass();

		void genFrameBuffer(const vulkan::Image* image);

		void render(vk::CommandBuffer cb);

		void renderDynamic(vk::CommandBuffer cb);
	};
}
