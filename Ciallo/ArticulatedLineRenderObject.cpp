#include "pch.hpp"
#include "ArticulatedLineRenderObject.hpp"

#include "ArticulatedLine.hpp"
#include "Stroke.hpp"
#include "Buffer.hpp"
#include "CtxUtilities.hpp"
#include "Tags.hpp"

namespace ciallo
{
	void ArticulatedLineRenderObjectAssigner::update(entt::registry& registry)
	{
		auto v = registry.view<Constructed<ArticulatedLineSettings>, StrokeCpo>();
		for (auto e : v)
		{
			auto* device = registry.ctx().at<vulkan::Device*>();
			vk::CommandBuffer cb = registry.ctx().at<CommandBuffers>().mainCb();
			auto& engine = registry.ctx().at<ArticulatedLineEngine>();
			auto& renderObjCpo = registry.emplace<ArticulatedLineRenderObjectCpo>(e);

			auto& polyline = registry.get<PolylineCpo>(e).polyline;
			vk::DeviceSize polylineBufferSize = sizeof(geom::Point) * polyline.size();
			vulkan::Buffer polylineBuffer{
				*device, vulkan::MemoryAuto, polylineBufferSize,
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
			};
			polylineBuffer.upload(cb, polyline.data(), VK_WHOLE_SIZE);
			renderObjCpo.vertexBuffers.push_back(std::move(polylineBuffer));

			auto& width = registry.get<WidthPerVertCpo>(e).width;
			vk::DeviceSize widthBufferSize = sizeof(float) * width.size();
			vulkan::Buffer widthBuffer{
				*device, vulkan::MemoryAuto, widthBufferSize,
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
			};
			widthBuffer.upload(cb, width.data(), VK_WHOLE_SIZE);
			renderObjCpo.vertexBuffers.push_back(std::move(widthBuffer));

			glm::vec4 color = registry.get<ColorCpo>(e).color;
			vulkan::Buffer colorBuffer{
				*device, vulkan::MemoryAuto, sizeof(glm::vec4),
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer
			};
			colorBuffer.upload(cb, &color, VK_WHOLE_SIZE);
			renderObjCpo.colorBuffer = std::move(colorBuffer);
			
			// renderObjCpo.objectDescriptorSet = device->createDescriptorSetUnique(layout);
		}
	}
}
