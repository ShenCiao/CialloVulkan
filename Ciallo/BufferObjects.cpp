#include "pch.hpp"
#include "BufferObjects.hpp"

#include <glm/gtx/optimum_pow.hpp>
#include <glm/gtc/integer.hpp>

#include "CtxUtilities.hpp"
#include "Stroke.hpp"

namespace ciallo
{
	void PolylineBuffer::connect(entt::registry& r)
	{
		ob.connect(r, entt::collector.update<PolylineCpo>().where<PolylineBuffer>());
		r.on_construct<PolylineBuffer>().connect<&onConstruct>();
	}

	void PolylineBuffer::updateBuffer(entt::registry& r, entt::entity e, vk::CommandBuffer cb,
	                                  const vulkan::Device* device)
	{
		vulkan::Buffer& buffer = r.get<PolylineBuffer>(e).buffer;
		auto& polyline = r.get<PolylineCpo>(e).polyline;
		size_t bufVertCount = buffer.size() / sizeof(geom::Point);

		if (!buffer.allocated() || polyline.size() > bufVertCount)
		{
			size_t n = glm::pow2<size_t>(glm::log2<size_t>(polyline.size()) + 1);
			buffer = vulkan::Buffer{
				*device, vulkan::MemoryAuto, n * sizeof(geom::Point),
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
			};
		}
		buffer.upload(cb, polyline.data(), polyline.size() * sizeof(geom::Point));
	}

	void PolylineBuffer::onConstruct(entt::registry& r, entt::entity e)
	{
		vk::CommandBuffer cb = r.ctx().at<CommandBuffers>().mainCb();
		auto* device = r.ctx().at<vulkan::Device*>();
		updateBuffer(r, e, cb, device);
	}

	void PolylineBuffer::update(entt::registry& r)
	{
		vk::CommandBuffer cb = r.ctx().at<CommandBuffers>().mainCb();
		auto* device = r.ctx().at<vulkan::Device*>();
		for (entt::entity e : ob)
		{
			updateBuffer(r, e, cb, device);
		}
	}

	void WidthPerVertBuffer::connect(entt::registry& r)
	{
		ob.connect(r, entt::collector.update<WidthPerVertCpo>().where<WidthPerVertBuffer>());
		r.on_construct<WidthPerVertBuffer>().connect<&onConstruct>();
	}

	void WidthPerVertBuffer::updateBuffer(entt::registry& r, entt::entity e, vk::CommandBuffer cb,
	                                      const vulkan::Device* device)
	{
		vulkan::Buffer& buffer = r.get<WidthPerVertBuffer>(e).buffer;
		auto& width = r.get<WidthPerVertCpo>(e).width;
		size_t bufVertCount = buffer.size() / sizeof(float);

		if (!buffer.allocated() || width.size() > bufVertCount)
		{
			size_t n = glm::pow2<size_t>(glm::log2<size_t>(width.size()) + 1);
			buffer = vulkan::Buffer{
				*device, vulkan::MemoryAuto, n * sizeof(float),
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
			};
		}
		buffer.upload(cb, width.data(), width.size() * sizeof(float));
	}

	void WidthPerVertBuffer::onConstruct(entt::registry& r, entt::entity e)
	{
		vk::CommandBuffer cb = r.ctx().at<CommandBuffers>().mainCb();
		auto* device = r.ctx().at<vulkan::Device*>();
		updateBuffer(r, e, cb, device);
	}

	void WidthPerVertBuffer::update(entt::registry& r)
	{
		vk::CommandBuffer cb = r.ctx().at<CommandBuffers>().mainCb();
		auto* device = r.ctx().at<vulkan::Device*>();
		for (entt::entity e : ob)
		{
			updateBuffer(r, e, cb, device);
		}
	}

	void ColorBuffer::connect(entt::registry& r)
	{
		ob.connect(r, entt::collector.update<ColorCpo>().where<ColorBuffer>());
		r.on_construct<ColorBuffer>().connect<&onConstruct>();
	}

	void ColorBuffer::updateBuffer(entt::registry& r, entt::entity e, vk::CommandBuffer cb,
	                               const vulkan::Device* device)
	{
		vulkan::Buffer& buffer = r.get<ColorBuffer>(e).buffer;
		glm::vec4 color = r.get<ColorCpo>(e).color;

		if (!buffer.allocated())
		{
			buffer = vulkan::Buffer{
				*device, vulkan::MemoryAuto, sizeof(glm::vec4),
				vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst
			};
		}
		buffer.upload(cb, &color, sizeof(glm::vec4));
	}

	void ColorBuffer::onConstruct(entt::registry& r, entt::entity e)
	{
		vk::CommandBuffer cb = r.ctx().at<CommandBuffers>().mainCb();
		auto* device = r.ctx().at<vulkan::Device*>();
		updateBuffer(r, e, cb, device);
	}

	void ColorBuffer::update(entt::registry& r)
	{
		vk::CommandBuffer cb = r.ctx().at<CommandBuffers>().mainCb();
		auto* device = r.ctx().at<vulkan::Device*>();
		for (entt::entity e : ob)
		{
			updateBuffer(r, e, cb, device);
		}
	}
}
