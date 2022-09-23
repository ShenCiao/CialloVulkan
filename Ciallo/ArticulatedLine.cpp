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
		auto& data = r.emplace<ArticulatedLineStrokeData>(e);

		// data.pipeline = *m_pipeline;
		// data.pipelineLayout = *m_pipelineLayout;
		// data.descriptorSetLayout = *m_descriptorSetLayout;
		// data.vertexCount = static_cast<uint32_t>(r.get<PolylineCpo>(e).polyline.size());

		vku::DescriptorSetUpdater updater;
		updater.beginDescriptorSet(*data.descriptorSet)
		       .buffer(r.get<ColorBuffer>(e).buffer)
		       .update(*device);

		r.patch<ArticulatedLineStrokeData>(e);
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


		vku::DescriptorSetLayoutMaker strokeDsMaker;
		strokeDescriptorSetLayout = strokeDsMaker.createUnique(device);

		vku::DescriptorSetLayoutMaker brushDsMaker;
		brushDsMaker.buffer(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1);
		brushDescriptorSetLayout = brushDsMaker.createUnique(device);

		// TODO: Ìí¼ÓÆäËûdescriptor set layout
		std::vector<vk::DescriptorSetLayout> layouts{*strokeDescriptorSetLayout, *brushDescriptorSetLayout};
		vk::PipelineLayoutCreateInfo info{{}, layouts, {}};
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

	void ArticulatedLineEngine::assignBrushRenderingData(entt::registry& r, entt::entity brushE,
	                                                     const ArticulatedLineSettings& settings)
	{
		// check out settings

		// set suitable buffer, renderer data and renderer
		r.emplace<ColorBuffer>(brushE);
		auto& brushData = r.emplace<ArticulatedLineBrushData>(brushE);
		auto* device = r.ctx().at<vulkan::Device*>();
		brushData.pipeline = *pipeline;
		brushData.pipelineLayout = *pipelineLayout;
		vku::DescriptorSetUpdater updater;
		updater.beginDescriptorSet(*brushData.descriptorSet)
		       .buffer(r.get<ColorBuffer>(brushE).buffer)
		       .update(*device);

		r.emplace<ArticulatedLineRenderMethodFlags>(brushE);
	}

	void ArticulatedLineEngine::assignStrokeRenderingData(entt::registry& r, entt::entity strokeE,
	                                                      const ArticulatedLineSettings& settings)
	{
		auto* device = r.ctx().at<vulkan::Device*>();
		auto& strokeData = r.emplace<ArticulatedLineStrokeData>(strokeE);

		r.emplace<PolylineBuffer>(strokeE);
		strokeData.vertexBufferIds.push_back(entt::type_id<PolylineBuffer>().hash());
		r.emplace<WidthPerVertBuffer>(strokeE);
		strokeData.vertexBufferIds.push_back(entt::type_id<WidthPerVertBuffer>().hash());

		strokeData.descriptorSet = device->createDescriptorSetUnique(*strokeDescriptorSetLayout);
	}

	void ArticulatedLineEngine::removeRenderingData(entt::registry& r, entt::entity e)
	{
		// TODO: Remove all buffers with a dedicate system
		
		r.remove<ArticulatedLineStrokeData>(e);
		r.remove<ArticulatedLineBrushData>(e);
		r.remove<ArticulatedLineRenderMethodFlags>(e);
	}

	void ArticulatedLineEngine::render(entt::registry& r, entt::entity brushE, entt::entity strokeE, vk::CommandBuffer cb)
	{
		auto& strokeData = r.get<ArticulatedLineStrokeData>(strokeE);
		auto& brushData = r.get<ArticulatedLineBrushData>(brushE);

		cb.bindPipeline(vk::PipelineBindPoint::eGraphics, brushData.pipeline);
		cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, brushData.pipelineLayout, 1, *brushData.descriptorSet, {});

		std::vector<vk::Buffer> vbs;
		for(auto id : strokeData.vertexBufferIds)
		{
			auto& base = r.storage(id)->second;
			auto* buffer = static_cast<vulkan::Buffer*>(base.get(brushE));
			vbs.push_back(*buffer);
		}
		cb.bindVertexBuffers({}, vbs, {});
		cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, brushData.pipelineLayout, 0, *strokeData.descriptorSet, {});
		uint32_t vertCount = r.get<PolylineCpo>(strokeE).polyline.size();
		cb.draw(vertCount, {}, {}, {});
	}
}
