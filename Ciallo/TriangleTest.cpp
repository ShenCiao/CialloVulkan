#include "pch.hpp"
#include "TriangleTest.hpp"

namespace ciallo::rendering
{
	TriangleTest::TriangleTest(vulkan::Device* device, vulkan::Image* target): m_device(*device), m_target(target)
	{
		m_vert = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eVertex, "./shaders/triangle.vert.spv");
		m_frag = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eFragment, "./shaders/triangle.frag.spv");
		genPipelineLayout();
		genRenderPass();
		genPipelineDynamic();
		genFrameBuffer(target);
	}

	void TriangleTest::genPipelineLayout()
	{
		vku::PipelineLayoutMaker maker;
		m_pipelineLayout = maker.createUnique(m_device);
	}

	void TriangleTest::genPipelineDynamic()
	{
		uint32_t width = m_target->width();
		uint32_t height = m_target->height();

		std::vector<vk::Format> colorAttachmentsFormats{vk::Format::eR8G8B8A8Unorm};
		vk::PipelineRenderingCreateInfo renderingCreateInfo{0, colorAttachmentsFormats};
		vku::PipelineMaker maker(0, 0);
		maker.dynamicState(vk::DynamicState::eViewport)
		     .dynamicState(vk::DynamicState::eScissor)
		     .shader(vk::ShaderStageFlagBits::eVertex, m_vert)
		     .shader(vk::ShaderStageFlagBits::eFragment, m_frag);
		m_pipeline = maker.createUnique(m_device, nullptr, *m_pipelineLayout, renderingCreateInfo);
	}

	void TriangleTest::genRenderPass()
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

	void TriangleTest::genFrameBuffer(const vulkan::Image* image)
	{
		auto imageView = image->imageView();
		vk::FramebufferCreateInfo info{{}, *m_renderPass, imageView, image->width(), image->height(), 1};
		m_framebuffer = m_device.createFramebufferUnique(info);
	}

	void TriangleTest::render(vk::CommandBuffer cb)
	{
		vk::RenderPassBeginInfo rpbi{*m_renderPass, *m_framebuffer, {{0, 0}, m_target->extent()}, {}};
		cb.beginRenderPass(rpbi, vk::SubpassContents::eInline);
		cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
		cb.draw(3, 1, 0, 0);
		cb.endRenderPass();
	}

	void TriangleTest::renderDynamic(vk::CommandBuffer cb)
	{
		vk::Rect2D area{{0, 0}, m_target->extent()};
		vk::RenderingAttachmentInfo renderingAttachmentInfo{m_target->imageView(), m_target->imageLayout()};
		std::vector colorAttachments{renderingAttachmentInfo};
		vk::RenderingInfo renderingInfo{{}, area, 1, 0, colorAttachments, {}, {}};
		cb.beginRendering(renderingInfo);
		vk::Viewport allViewport{0, 0, static_cast<float>(m_target->width()), static_cast<float>(m_target->height()), 0.0f, 1.0f};
		cb.setViewport(0, allViewport);
		vk::Rect2D allScissor{{0, 0}, m_target->extent()};
		cb.setScissor(0, allScissor);
		cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
		cb.draw(3, 1, 0, 0);
		cb.endRendering();
	}
}
