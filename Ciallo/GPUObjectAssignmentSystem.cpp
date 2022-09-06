#include "pch.hpp"
#include "GPUObjectAssignmentSystem.hpp"

#include "Stroke.hpp"
#include "Buffer.hpp"

namespace ciallo
{
	void GPUObjectAssignmentSystem::start(entt::registry& registry)
	{
		registry.on_construct<PolylineCpo>().connect<&onPolylineCpoChanged>();

	}

	void GPUObjectAssignmentSystem::onPolylineCpoChanged(entt::registry& registry, entt::entity e)
	{
		auto& cpo = registry.get_or_emplace<GPUBufferCpo>(e);
		vulkan::Buffer& buffer = cpo.buffer;
	}
}
