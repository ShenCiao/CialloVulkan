#include "pch.hpp"
#include "ArticulatedLine.hpp"

#include "vku.hpp"
#include "ArticulatedLineRenderer.hpp"
#include "Stroke.hpp"
#include "StrokeBufferObjects.hpp"

namespace ciallo
{
	ArticulatedLineEngineTemp::ArticulatedLineEngineTemp(vulkan::Device* device): m_device(*device)
	{
		m_vertShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eVertex,
		                                    "./shaders/articulatedLineTemp.vert.spv");
		m_fragShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eFragment,
		                                    "./shaders/articulatedLineTemp.frag.spv");
		m_geomShader = vulkan::ShaderModule(*device, vk::ShaderStageFlagBits::eGeometry,
		                                    "./shaders/articulatedLineTemp.geom.spv");
		genPipelineLayout();
		genPipelineDynamic();
		genVertexBuffer(*device);
	}

	void ArticulatedLineEngineTemp::genPipelineLayout()
	{
		vku::PipelineLayoutMaker maker;
		m_pipelineLayout = maker.createUnique(m_device);
	}

	void ArticulatedLineEngineTemp::genPipelineDynamic()
	{
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
		     .vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
		     .vertexAttribute(1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color))
		     .vertexAttribute(2, 0, vk::Format::eR32Sfloat, offsetof(Vertex, width));
		m_pipeline = maker.createUnique(m_device, nullptr, *m_pipelineLayout, renderingCreateInfo);
	}

	void ArticulatedLineEngineTemp::renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target)
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

	void ArticulatedLineEngineTemp::genVertexBuffer(VmaAllocator allocator)
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

	void ArticulatedLineEngineTemp::assignRenderer(entt::registry& r, entt::entity e, vulkan::Device* device)
	{
		// check out ArticulatedLineSettings

		// set suitable renderer data and renderer
		auto& data = r.emplace<ArticulatedLineRenderingData>(e);

		data.pipeline = *m_pipeline;
		data.pipelineLayout = *m_pipelineLayout;
		data.descriptorSetLayout = *m_descriptorSetLayout;
		data.vertexCount = static_cast<uint32_t>(r.get<PolylineCpo>(e).polyline.size());

		vku::DescriptorSetUpdater updater;
		updater.beginDescriptorSet(*data.entityDescriptorSet)
		       .buffer(r.get<ColorBuffer>(e).buffer)
		       .update(*device);

		r.patch<ArticulatedLineRenderingData>(e);
	}

	void ArticulatedLineEngineTemp::connect(entt::registry& r)
	{
		obs[0].connect(r, entt::collector.update<ArticulatedLineSettings>());
	}

	void ArticulatedLineEngineTemp::update(entt::registry& r)
	{
		auto* device = r.ctx().at<vulkan::Device*>();
		auto& engine = r.ctx().at<ArticulatedLineEngineTemp>();
		for (auto e : obs[0])
		{
			engine.assignRenderer(r, e, device);
		}
	}

	ArticulatedLineEngine::ArticulatedLineEngine(vk::Device device)
	{
		init(device);
	}

	void ArticulatedLineEngine::init(vk::Device device)
	{
		auto vertShaderData = vulkan::ShaderModule::loadSpv("./shaders/articulatedLine.vert.spv");
		vk::ShaderModuleCreateInfo vertInfo{
			{}, vertShaderData.size(), reinterpret_cast<const uint32_t*>(vertShaderData.data())
		};
		vertShader = device.createShaderModuleUnique(vertInfo);

		auto geomShaderData = vulkan::ShaderModule::loadSpv("./shaders/articulatedLine.geom.spv");
		vk::ShaderModuleCreateInfo geomInfo{
			{}, geomShaderData.size(), reinterpret_cast<const uint32_t*>(geomShaderData.data())
		};
		geomShader = device.createShaderModuleUnique(geomInfo);

		auto fragShaderData = vulkan::ShaderModule::loadSpv("./shaders/articulatedLine.frag.spv");
		vk::ShaderModuleCreateInfo fragInfo{
			{}, fragShaderData.size(), reinterpret_cast<const uint32_t*>(fragShaderData.data())
		};
		fragShader = device.createShaderModuleUnique(fragInfo);

		vku::DescriptorSetLayoutMaker dscripMaker;
		dscripMaker.buffer(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1);
		entityDescriptorSetLayout = dscripMaker.createUnique(device);

		// TODO: Ìí¼ÓÆäËûdescriptor set layout
		vk::PipelineLayoutCreateInfo info{{}, *entityDescriptorSetLayout, {}};
		pipelineLayout = device.createPipelineLayoutUnique(info);

		std::vector<vk::Format> colorAttachmentsFormats{vk::Format::eR8G8B8A8Unorm};
		vk::PipelineRenderingCreateInfo renderingCreateInfo{0, colorAttachmentsFormats};
		vku::PipelineMaker maker;
		maker.topology(vk::PrimitiveTopology::eLineStrip)
		     .dynamicState(vk::DynamicState::eViewport)
		     .dynamicState(vk::DynamicState::eScissor)
		     .shader(vk::ShaderStageFlagBits::eVertex, *vertShader)
		     .shader(vk::ShaderStageFlagBits::eFragment, *fragShader)
		     .shader(vk::ShaderStageFlagBits::eGeometry, *geomShader)
		     .blendEnable(VK_TRUE)
		     .cullMode(vk::CullModeFlagBits::eNone)
		     .vertexBinding(0, sizeof(geom::Point))
		     .vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
		     .vertexBinding(1, sizeof(float))
		     .vertexAttribute(1, 1, vk::Format::eR32Sfloat, 0);
		pipeline = maker.createUnique(device, nullptr, *pipelineLayout, renderingCreateInfo);
	}

	void ArticulatedLineEngine::connect(entt::registry& r)
	{
		obs[0].connect(r, entt::collector.update<ArticulatedLineSettings>());
		r.on_destroy<ArticulatedLineSettings>().connect<&ArticulatedLineEngine::removeRenderer>(*this);
	}

	void ArticulatedLineEngine::assignRenderer(entt::registry& r, entt::entity e)
	{
		// check out ArticulatedLineSettings

		// set suitable buffer, renderer data and renderer
		r.emplace<PolylineBuffer>(e);
		r.emplace<WidthPerVertBuffer>(e);
		r.emplace<ColorBuffer>(e);
		auto& data = r.emplace<ArticulatedLineRenderingData>(e);
		auto* device = r.ctx().at<vulkan::Device*>();
		data.pipeline = *pipeline;
		data.pipelineLayout = *pipelineLayout;
		data.descriptorSetLayout = *entityDescriptorSetLayout;
		data.vertexCount = static_cast<uint32_t>(r.get<PolylineCpo>(e).polyline.size());
		data.entityDescriptorSet = device->createDescriptorSetUnique(*entityDescriptorSetLayout);

		vku::DescriptorSetUpdater updater;
		updater.beginDescriptorSet(*data.entityDescriptorSet)
		       .buffer(r.get<ColorBuffer>(e).buffer)
		       .update(*device);

		r.patch<ArticulatedLineRenderingData>(e);
		r.emplace<ArticulatedLineDefaultRenderTag>(e);
	}

	void ArticulatedLineEngine::removeRenderer(entt::registry& r, entt::entity e)
	{
		// TODO: Remove all buffers with a dedicate system
		r.remove<ArticulatedLineRenderingData>(e);
		r.remove<ArticulatedLineDefaultRenderTag>(e);
	}
}
