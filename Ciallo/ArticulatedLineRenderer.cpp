#include "pch.hpp"
#include "ArticulatedLineRenderer.hpp"

#include "Stroke.hpp"
#include "BufferObjects.hpp"
#include "vku.hpp"

namespace ciallo
{
	void ArticulatedLineRenderingData::connect(entt::registry& r)
	{
		ob.connect(r, entt::collector.update<PolylineCpo>().where<ArticulatedLineRenderingData>());
	}

	void ArticulatedLineRenderingData::update(entt::registry& r)
	{
		for (entt::entity e : ob)
		{
			r.patch<ArticulatedLineRenderingData>(e, [&](ArticulatedLineRenderingData& data)
			{
				data.vertexCount = static_cast<uint32_t>(r.get<PolylineCpo>(e).polyline.size());
			});
		}
	}

	void ArticulatedLineDefaultRenderer::render(entt::registry& r, entt::entity e, vk::CommandBuffer cb)
	{
		std::vector<vk::Buffer> vbs(2);
		vbs.push_back(r.get<PolylineBuffer>(e).buffer);
		vbs.push_back(r.get<WidthPerVertBuffer>(e).buffer);

		auto& d = r.get<ArticulatedLineRenderingData>(e);

		cb.bindVertexBuffers(0, vbs, {0});
		cb.bindPipeline(vk::PipelineBindPoint::eGraphics, d.pipeline);
		cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, d.pipelineLayout, 0, *d.objectDescriptorSet, nullptr);
		cb.draw(d.vertexCount, 1, 0, 0);
	}
}
