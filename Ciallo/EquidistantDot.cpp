#include "pch.hpp"
#include "EquidistantDot.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "vku.hpp"

namespace ciallo
{
	EquidistantDotEngine::EquidistantDotEngine(vulkan::Device* device): m_device(*device)
	{
		m_compShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eCompute,
		                                    "./shaders/equidistantDot.comp.spv");
		m_vertShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eVertex,
		                                    "./shaders/equidistantDot.vert.spv");
		m_fragShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eFragment,
		                                    "./shaders/equidistantDot.frag.spv");
		m_geomShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eGeometry,
		                                    "./shaders/equidistantDot.geom.spv");
		genInputBuffer(*device);
		genAuxiliaryBuffer(*device);
		genCompDescriptorSet(device->descriptorPool());
		genCompPipeline();
		genPipelineDynamic();
	}

	void EquidistantDotEngine::genPipelineDynamic()
	{
		vku::PipelineLayoutMaker layoutMaker;
		m_pipelineLayout = layoutMaker.createUnique(m_device);

		std::vector<vk::Format> colorAttachmentsFormats{vk::Format::eR8G8B8A8Unorm};
		vk::PipelineRenderingCreateInfo renderingCreateInfo{0, colorAttachmentsFormats};
		vku::PipelineMaker maker;
		maker.topology(vk::PrimitiveTopology::eLineStrip)
		     .dynamicState(vk::DynamicState::eViewport)
		     .dynamicState(vk::DynamicState::eScissor)
		     .shader(vk::ShaderStageFlagBits::eVertex, m_vertShader)
		     .shader(vk::ShaderStageFlagBits::eFragment, m_fragShader)
		     .shader(vk::ShaderStageFlagBits::eGeometry, m_geomShader)
		     .blendEnable(VK_TRUE)
		     .cullMode(vk::CullModeFlagBits::eNone)
		     .vertexBinding(0, sizeof(Vertex))
		     .vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos))
		     .vertexAttribute(1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color))
		     .vertexAttribute(2, 0, vk::Format::eR32Sfloat, offsetof(Vertex, width));
		m_pipeline = maker.createUnique(m_device, nullptr, *m_pipelineLayout, renderingCreateInfo);
	}

	void EquidistantDotEngine::genCompDescriptorSet(vk::DescriptorPool pool)
	{
		vku::DescriptorSetLayoutMaker layoutMaker;
		layoutMaker.buffer(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
		           .buffer(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
		           .buffer(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
		           .buffer(3, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute, 1);
		m_compDescriptorSetLayout = layoutMaker.createUnique(m_device);

		vku::DescriptorSetMaker maker;
		maker.layout(*m_compDescriptorSetLayout);
		auto descriptorSets = maker.create(m_device, pool);
		m_compDescriptorSet = descriptorSets[0];

		vku::DescriptorSetUpdater updater;
		updater.beginDescriptorSet(m_compDescriptorSet)
		       .beginBuffers(0, 0, vk::DescriptorType::eStorageBuffer)
		       .buffer(m_inputBuffer)
		       .beginBuffers(1, 0, vk::DescriptorType::eStorageBuffer)
		       .buffer(m_vertBuffer)
		       .beginBuffers(2, 0, vk::DescriptorType::eStorageBuffer)
		       .buffer(m_indirectDrawBuffer)
		       .beginBuffers(3, 0, vk::DescriptorType::eUniformBuffer)
		       .buffer(m_tempBufferForSpacing);
		updater.update(m_device);
	}

	// gen compute layout and pipeline
	void EquidistantDotEngine::genCompPipeline()
	{
		vku::PipelineLayoutMaker layoutMaker;
		layoutMaker.descriptorSetLayout(*m_compDescriptorSetLayout);
		m_compPipelineLayout = layoutMaker.createUnique(m_device);

		vku::ComputePipelineMaker maker{};
		maker.shader(vk::ShaderStageFlagBits::eCompute, m_compShader);
		m_compPipeline = maker.createUnique(m_device, nullptr, *m_compPipelineLayout);
	}

	void EquidistantDotEngine::renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target)
	{
		m_inputBuffer.uploadLocal(vertices.data(), VK_WHOLE_SIZE);
		m_tempBufferForSpacing.uploadLocal(&spacing, VK_WHOLE_SIZE);
		compute(cb);
		vk::MemoryBarrier2 drawIndirectBarrier{
			vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderStorageWrite,
			vk::PipelineStageFlagBits2::eDrawIndirect, vk::AccessFlagBits2::eIndirectCommandRead
		};
		vk::MemoryBarrier2 vertexBarrier{
			vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderStorageWrite,
			vk::PipelineStageFlagBits2::eVertexInput, vk::AccessFlagBits2::eVertexAttributeRead
		};

		std::vector barriers = {drawIndirectBarrier, vertexBarrier};
		cb.pipelineBarrier2({{}, barriers, {}, {}});

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
		cb.drawIndirect(m_indirectDrawBuffer, 0, 1, {});
		cb.endRendering();
	}

	void EquidistantDotEngine::genInputBuffer(VmaAllocator allocator)
	{
		vertices = {
			{{0.0f, 0.5f}, 0.01f, {}, {1.0f, 0.0f, 0.0f, 1.0f}},
			{{-glm::sqrt(3.0f) * 0.25f, -0.25f}, 0.01f, {}, {0.0f, 0.0f, 1.0f, 1.0f}},
			{{glm::sqrt(3.0f) * 0.25f, -0.25f}, 0.01f, {}, {0.0f, 1.0f, 0.0f, 1.0f}},
			{{0.0f, 0.5f}, 0.01f, {}, {1.0f, 0.0f, 0.0f, 1.0f}},
		};

		VmaAllocationCreateInfo info{VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO};
		auto size = vertices.size() * sizeof(Vertex);
		m_inputBuffer = vulkan::Buffer(allocator, info, size, vk::BufferUsageFlagBits::eStorageBuffer);
		m_inputBuffer.uploadLocal(vertices.data(), size);

		m_tempBufferForSpacing = vulkan::Buffer(allocator, info, sizeof(float), vk::BufferUsageFlagBits::eUniformBuffer);
		m_tempBufferForSpacing.uploadLocal(&spacing, VK_WHOLE_SIZE);
	}

	void EquidistantDotEngine::genAuxiliaryBuffer(VmaAllocator allocator)
	{
		VmaAllocationCreateInfo info{{}, VMA_MEMORY_USAGE_AUTO};
		constexpr int MAX_DOT = 1024 * 64;
		auto size = MAX_DOT * sizeof(Vertex);
		m_vertBuffer = vulkan::Buffer(allocator, info, size,
		                              vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer);

		m_indirectDrawBuffer = vulkan::Buffer(allocator, info, sizeof(vk::DrawIndirectCommand),
		                                      vk::BufferUsageFlagBits::eStorageBuffer |
		                                      vk::BufferUsageFlagBits::eIndirectBuffer);
	}

	void EquidistantDotEngine::compute(vk::CommandBuffer cb)
	{
		cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_compPipelineLayout, 0, m_compDescriptorSet,
		                      nullptr);
		cb.bindPipeline(vk::PipelineBindPoint::eCompute, *m_compPipeline);
		cb.dispatch(1, 1, 1);
	}

	void EquidistantDotRenderer::render(vk::CommandBuffer cb)
	{
		// Compute
	}
}
