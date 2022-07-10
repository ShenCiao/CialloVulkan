#include "pch.hpp"
#include "TriangleTest.hpp"

#include <memory>
#include <glm/glm.hpp>

namespace ciallo::rendering
{
	TriangleTest::TriangleTest(vulkan::Device* device, vulkan::Image* target): m_device(*device), m_target(target)
	{
		m_vertShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eVertex, "./shaders/triangle.vert.spv");
		m_fragShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eFragment, "./shaders/triangle.frag.spv");
		m_geomShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eGeometry, "./shaders/triangle.geom.spv");
		genPipelineLayout();
		genPipelineDynamic();
		genVertexBuffer(*device);
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
		maker.topology(vk::PrimitiveTopology::eTriangleStrip)
		     .dynamicState(vk::DynamicState::eViewport)
		     .dynamicState(vk::DynamicState::eScissor)
		     .shader(vk::ShaderStageFlagBits::eVertex, m_vertShader)
		     .shader(vk::ShaderStageFlagBits::eFragment, m_fragShader)
		     .shader(vk::ShaderStageFlagBits::eGeometry, m_geomShader)
		     .cullMode(vk::CullModeFlagBits::eNone)
		     .vertexBinding(0, 5 * sizeof(float))
		     .vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
		     .vertexAttribute(1, 0, vk::Format::eR32G32B32Sfloat, vk::blockSize(vk::Format::eR32G32Sfloat));
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
		vk::Viewport allViewport{
			0, 0, static_cast<float>(m_target->width()), static_cast<float>(m_target->height()), 0.0f, 1.0f
		};
		cb.setViewport(0, allViewport);
		vk::Rect2D allScissor{{0, 0}, m_target->extent()};
		cb.setScissor(0, allScissor);
		std::vector<vk::Buffer> vertexBuffers{m_vertBuffer};
		cb.bindVertexBuffers(0, vertexBuffers, {0});
		cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
		cb.draw(3, 1, 0, 0);
		cb.endRendering();
	}

	void TriangleTest::genVertexBuffer(VmaAllocator allocator)
	{
		struct Vertex
		{
			glm::vec2 pos;
			glm::vec3 color;
		};

		const std::vector<Vertex> vertices = {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};
		VmaAllocationCreateInfo info{VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO};

		auto size = vertices.size() * sizeof(Vertex);
		m_vertBuffer = vulkan::Buffer(allocator, info, size, vk::BufferUsageFlagBits::eVertexBuffer);
		m_vertBuffer.uploadLocal(vertices.data(), size);
	}
}
