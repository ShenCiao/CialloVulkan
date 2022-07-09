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
		TriangleTest(vulkan::Device* device, vulkan::Image* target): m_device(*device), m_target(target)
		{
			m_vert = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eVertex, "./shaders/triangle.vert.spv");
			m_frag = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eFragment, "./shaders/triangle.frag.spv");
			genPipelineLayout();
			genRenderPass();
			genPipeline(target->width(), target->height());
			genFrameBuffer(target);
		}

		void genPipelineLayout()
		{
			vku::PipelineLayoutMaker maker;
			m_pipelineLayout = maker.createUnique(m_device);
		}

		void genPipeline(uint32_t width, uint32_t height)
		{
			vku::PipelineMaker maker(width, height);
			maker.shader(vk::ShaderStageFlagBits::eVertex, m_vert)
			     .shader(vk::ShaderStageFlagBits::eFragment, m_frag);
			m_pipeline = maker.createUnique(m_device, nullptr, *m_pipelineLayout, *m_renderPass);
		}

		void genRenderPass()
		{
			std::vector<vk::AttachmentDescription> attachments;
			std::vector<vk::SubpassDescription> subpasses;

			vk::AttachmentDescription attachment{{}, vk::Format::eR8G8B8A8Unorm};
			attachment.setInitialLayout(vk::ImageLayout::eGeneral);
			attachment.setFinalLayout(vk::ImageLayout::eGeneral);
			attachments.push_back(attachment);
			std::vector<vk::AttachmentReference> colorAttachments;
			colorAttachments.push_back({0, vk::ImageLayout::eGeneral});
			subpasses.push_back({{}, vk::PipelineBindPoint::eGraphics, {}, colorAttachments, {}, {}, {}});

			vk::RenderPassCreateInfo info{
				{},
				attachments,
				subpasses,
				{}
			};
			m_renderPass = m_device.createRenderPassUnique(info);
		}

		void genFrameBuffer(const vulkan::Image* image)
		{
			auto imageView = image->imageView();
			vk::FramebufferCreateInfo info{{}, *m_renderPass, imageView, image->width(), image->height(), 1};
			m_framebuffer = m_device.createFramebufferUnique(info);
		}

		void render(vk::CommandBuffer cb)
		{
			vk::RenderPassBeginInfo rpbi{*m_renderPass, *m_framebuffer, {{0, 0}, m_target->extent()}, {}};
			cb.beginRenderPass(rpbi, vk::SubpassContents::eInline);
			cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
			cb.draw(3, 1, 0, 0);
			cb.endRenderPass();
		}
	};
}
