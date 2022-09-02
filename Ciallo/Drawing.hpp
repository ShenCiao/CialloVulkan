#pragma once

namespace ciallo
{
	struct DrawingCpo
	{
		glm::vec2 min; // left upper corner of world coordinate, unit is meter.
		glm::vec2 max;
		float dpi = 74.0f;
		std::vector<entt::entity> layers{};

		float width() const // in pixel
		{
			return (max-min).x * dpi/0.0254f;
		}

		float height() const
		{
			return (max-min).y * dpi/0.0254f;
		}
	};

	constexpr DrawingCpo A4Paper{{0.0f, 0.0f}, {0.297f, 0.21f}, 144.0f};
}
