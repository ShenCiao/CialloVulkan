#include "pch.hpp"
#include "ArticulatedLineRenderer.hpp"

#include "Buffer.hpp"
#include "Tags.hpp"
#include "Stroke.hpp"

#include <glm/gtc/integer.hpp>

#include "CtxUtilities.hpp"

namespace ciallo
{
	void ArticulatedLineRenderer::render(vk::CommandBuffer cb)
	{
		std::vector<vk::Buffer> vbs{};
		for (vulkan::Buffer& buffer : vertexBuffers)
		{
			vbs.push_back(buffer);
		}
		cb.bindVertexBuffers(0, vbs, {0});
		cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *objectDescriptorSet, nullptr);
		cb.draw(vertexCount, 1, 0, 0);
	}

	void ArticulatedLineRenderer::update(entt::registry& r)
	{
		for (entt::entity e : r.view<Constructed<ArticulatedLineRenderer>>())
		{
			vk::CommandBuffer cb = r.ctx().at<CommandBuffers>().mainCb();
			auto* device = r.ctx().at<vulkan::Device*>();
			auto& renderer = r.get<ArticulatedLineRenderer>(e);

			std::vector<geom::Point>& polyline = r.get<PolylineCpo>(e).polyline;
			renderer.vertexCount = static_cast<uint32_t>(polyline.size());
			// nearest 2^n
			uint32_t memoryVertCount = glm::pow<uint32_t>(2u, glm::log2<uint32_t>(renderer.vertexCount) + 1);

			vulkan::Buffer polylineBuffer{
				*device, vulkan::MemoryAuto, memoryVertCount * sizeof(geom::Point),
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
			};
			polylineBuffer.upload(cb, polyline.data(), memoryVertCount * sizeof(geom::Point));
			renderer.vertexBuffers.push_back(std::move(polylineBuffer));

			std::vector<float>& width = r.get<WidthPerVertCpo>(e).width;
			vulkan::Buffer widthBuffer{
				*device, vulkan::MemoryAuto, memoryVertCount * sizeof(float),
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
			};
			widthBuffer.upload(cb, width.data(), memoryVertCount * sizeof(float));
			renderer.vertexBuffers.push_back(std::move(widthBuffer));

			glm::vec4 color = r.get<ColorCpo>(e).color;
			vulkan::Buffer colorBuffer{
				*device, vulkan::MemoryHostVisible, sizeof(glm::vec4),
				vk::BufferUsageFlagBits::eUniformBuffer
			};
			colorBuffer.upload(cb, &color, sizeof(glm::vec4));
			renderer.colorBuffer = std::move(colorBuffer);
			renderer.objectDescriptorSet = device->createDescriptorSetUnique(renderer.descriptorSetLayout);
		}
	}
}
