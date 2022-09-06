#pragma once

namespace ciallo
{
	class GPUObjectAssignmentSystem
	{
		static void start(entt::registry& registry);
		static void update(entt::registry& registry);
		static void onPolylineCpoChanged(entt::registry& registry, entt::entity e);
	};
}
