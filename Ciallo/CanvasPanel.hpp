#pragma once

namespace ciallo
{
	struct CanvasPanelCpo
	{
		entt::entity drawing{entt::null};
		float drawingRotation = 0.0f;
		float zoom = 1.0f;
		glm::vec2 scroll{0.0f, 0.0f};
		std::vector<entt::entity> onionSkinDrawings = {};
		std::vector<float> onionSkinDrawingRotations = {};
	};

	struct CanvasPanelDrawer
	{
		static void update(entt::registry& registry);
	};
}
