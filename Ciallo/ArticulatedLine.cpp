#include "pch.hpp"
#include "ArticulatedLine.hpp"

#include "vku.hpp"
#include "CommonSceneObjectComponents.hpp"

namespace ciallo::rendering
{
	ArticulatedLineEngine::ArticulatedLineEngine(vulkan::Device* device): m_device(*device)
	{
		m_vertShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eVertex,
		                                    "./shaders/articulatedLine.vert.spv");
		m_fragShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eFragment,
		                                    "./shaders/articulatedLine.frag.spv");
		m_geomShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eGeometry,
		                                    "./shaders/articulatedLine.geom.spv");
		genPipelineLayout();
		genPipelineDynamic();
		genVertexBuffer(*device);
	}

	void ArticulatedLineEngine::genPipelineLayout()
	{
		vku::PipelineLayoutMaker maker;
		m_pipelineLayout = maker.createUnique(m_device);
	}

	void ArticulatedLineEngine::genPipelineDynamic()
	{
		std::vector<vk::Format> colorAttachmentsFormats{vk::Format::eR8G8B8A8Unorm};
		vk::PipelineRenderingCreateInfo renderingCreateInfo{0, colorAttachmentsFormats};
		vku::PipelineMaker maker(0, 0);
		maker.topology(vk::PrimitiveTopology::eLineStrip)
		     .dynamicState(vk::DynamicState::eViewport)
		     .dynamicState(vk::DynamicState::eScissor)
		     .shader(vk::ShaderStageFlagBits::eVertex, m_vertShader)
		     .shader(vk::ShaderStageFlagBits::eFragment, m_fragShader)
		     .shader(vk::ShaderStageFlagBits::eGeometry, m_geomShader)
		     .blendEnable(VK_TRUE)
		     .cullMode(vk::CullModeFlagBits::eNone)
		     .vertexBinding(0, sizeof(Vertex))
		     .vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
		     .vertexAttribute(1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color))
		     .vertexAttribute(2, 0, vk::Format::eR32Sfloat, offsetof(Vertex, width));
		m_pipeline = maker.createUnique(m_device, nullptr, *m_pipelineLayout, renderingCreateInfo);
	}

	void ArticulatedLineEngine::renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target)
	{
		m_vertBuffer.uploadLocal(vertices.data(), VK_WHOLE_SIZE);
		vk::Rect2D area{{0, 0}, target->extent2D()};
		vk::RenderingAttachmentInfo renderingAttachmentInfo{target->imageView(), target->imageLayout()};
		std::vector colorAttachments{renderingAttachmentInfo};
		vk::RenderingInfo renderingInfo{{}, area, 1, 0, colorAttachments, {}, {}};
		cb.beginRendering(renderingInfo);
		vk::Viewport fullViewport{
			0, 0, static_cast<float>(target->width()), static_cast<float>(target->height()), 0.0f, 1.0f
		};
		cb.setViewport(0, fullViewport);
		vk::Rect2D zeroScissor{{0, 0}, target->extent2D()};
		cb.setScissor(0, zeroScissor);
		std::vector<vk::Buffer> vertexBuffers{m_vertBuffer};
		cb.bindVertexBuffers(0, vertexBuffers, {0});
		cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
		cb.draw(4, 1, 0, 0);
		cb.endRendering();
	}

	void ArticulatedLineEngine::genVertexBuffer(VmaAllocator allocator)
	{
		vertices = {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, 0.02f},
			{{-glm::sqrt(3.0f) * 0.25f, 0.25f}, {0.0f, 0.0f, 1.0f, 1.0f}, 0.02f},
			{{glm::sqrt(3.0f) * 0.25f, 0.25f}, {0.0f, 1.0f, 0.0f, 1.0f}, 0.02f},
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, 0.02f},
		};
		VmaAllocationCreateInfo info{VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO};

		auto size = vertices.size() * sizeof(Vertex);
		m_vertBuffer = vulkan::Buffer(allocator, info, size, vk::BufferUsageFlagBits::eVertexBuffer);
		m_vertBuffer.uploadLocal(vertices.data(), size);
	}

	void ArticulatedLineRenderer::render(vk::CommandBuffer cb, entt::handle object)
	{
		auto& [buffers, vertexCount] = object.get<scene::VertexBufferCpo>();
		auto& [descriptorSet] = object.get<scene::PipelineResourceCpo>();
		vk::DescriptorSet desS = *descriptorSet;
		auto getBufferObject = [](const vulkan::Buffer& b)
		{
			return b.buffer();
		};
		std::vector<vk::Buffer> vbs = buffers | views::transform(getBufferObject) | ranges::to_vector;

		cb.bindVertexBuffers(0, vbs, {0});
		cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, desS, nullptr);
		cb.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
		cb.draw(vertexCount, 1, 0, 0);
	}
}
