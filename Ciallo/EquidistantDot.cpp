#include "pch.hpp"
#include "EquidistantDot.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "vku.hpp"

namespace ciallo::rendering
{
	EquidistantDot::EquidistantDot(vulkan::Device* device): m_device(*device)
	{
		m_computeShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eCompute,
		                                       "./shaders/equidistantDot.comp.spv");
		genInputBuffer(*device);
		genVertexBuffer(*device);
		genComputeDescriptorSet(device->descriptorPool());
		genComputePipeline();
	}

	void EquidistantDot::genPipelineLayout()
	{
		vku::PipelineLayoutMaker maker;
		m_pipelineLayout = maker.createUnique(m_device);
	}

	void EquidistantDot::genPipelineDynamic()
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
		     .cullMode(vk::CullModeFlagBits::eNone)
		     .vertexBinding(0, 5 * sizeof(float))
		     .vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
		     .vertexAttribute(1, 0, vk::Format::eR32G32B32Sfloat, vk::blockSize(vk::Format::eR32G32Sfloat));
		m_pipeline = maker.createUnique(m_device, nullptr, *m_pipelineLayout, renderingCreateInfo);
	}

	void EquidistantDot::genComputeDescriptorSet(vk::DescriptorPool pool)
	{
		vku::DescriptorSetLayoutMaker layoutMaker;
		layoutMaker.buffer(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
		           .buffer(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1);
		m_computeDescriptorSetLayout = layoutMaker.createUnique(m_device);

		vku::DescriptorSetMaker maker;
		maker.layout(*m_computeDescriptorSetLayout);
		auto descriptorSets = maker.create(m_device, pool);
		m_computeDescriptorSet = descriptorSets[0];

		vku::DescriptorSetUpdater updater;
		updater.beginDescriptorSet(m_computeDescriptorSet)
		       .beginBuffers(0, 0, vk::DescriptorType::eStorageBuffer)
		       .buffer(m_inputBuffer, 0, VK_WHOLE_SIZE)
		       .beginBuffers(1, 0, vk::DescriptorType::eStorageBuffer)
		       .buffer(m_dotBuffer, 0, VK_WHOLE_SIZE);
		updater.update(m_device);
	}

	// gen compute layout and pipeline
	void EquidistantDot::genComputePipeline()
	{
		vku::PipelineLayoutMaker layoutMaker;
		layoutMaker.descriptorSetLayout(*m_computeDescriptorSetLayout);
		m_computePipelineLayout = layoutMaker.createUnique(m_device);

		vku::ComputePipelineMaker maker{};
		maker.shader(vk::ShaderStageFlagBits::eCompute, m_computeShader);
		m_computePipeline = maker.createUnique(m_device, nullptr, *m_computePipelineLayout);
	}

	void EquidistantDot::renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target)
	{
		vk::Rect2D area{{0, 0}, target->extent()};
		vk::RenderingAttachmentInfo renderingAttachmentInfo{target->imageView(), target->imageLayout()};
		std::vector colorAttachments{renderingAttachmentInfo};
		vk::RenderingInfo renderingInfo{{}, area, 1, 0, colorAttachments, {}, {}};
		cb.beginRendering(renderingInfo);
		vk::Viewport fullViewport{
			0, 0, static_cast<float>(target->width()), static_cast<float>(target->height()), 0.0f, 1.0f
		};
		cb.setViewport(0, fullViewport);
		vk::Rect2D zeroScissor{{0, 0}, target->extent()};
		cb.setScissor(0, zeroScissor);
		std::vector<vk::Buffer> vertexBuffers{m_dotBuffer};
		cb.bindVertexBuffers(0, vertexBuffers, {0});
		cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
		cb.draw(4, 1, 0, 0);
		cb.endRendering();
	}

	struct Vertex
	{
		glm::vec2 pos;
		float pressure;
		float _pad0;
	};

	void EquidistantDot::genInputBuffer(VmaAllocator allocator)
	{
		std::vector<Vertex> vertices{};
		int n = 16;
		vertices.reserve(n);
		for (float i : views::iota(0, n))
		{
			float x = glm::mix(-1.0f, 1.0f, i/n);
			float y = x;
			vertices.push_back({{x, y}, 1.0f, {}});
		}


		VmaAllocationCreateInfo info{VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO};
		auto size = vertices.size() * sizeof(Vertex);
		m_inputBuffer = vulkan::Buffer(allocator, info, size, vk::BufferUsageFlagBits::eStorageBuffer);
		m_inputBuffer.uploadLocal(vertices.data(), size);
	}

	void EquidistantDot::genVertexBuffer(VmaAllocator allocator)
	{
		VmaAllocationCreateInfo info{{}, VMA_MEMORY_USAGE_AUTO};
		constexpr int MAX_DOT = 1024 * 8;
		auto size = MAX_DOT * sizeof(Vertex);
		m_dotBuffer = vulkan::Buffer(allocator, info, size,
		                             vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer);
	}

	void EquidistantDot::compute(vk::CommandBuffer cb)
	{
		cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_computePipelineLayout, 0, m_computeDescriptorSet,
		                      nullptr);
		cb.bindPipeline(vk::PipelineBindPoint::eCompute, *m_computePipeline);
		cb.dispatch(1, 1, 1);
	}
}
