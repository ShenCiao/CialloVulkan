#pragma once

namespace ciallo
{
	struct DrawingTag
	{
	};

	struct ViewRectCpo
	{
		glm::vec2 min; // left upper corner of world coordinate, unit is meter.
		glm::vec2 max;
		float dpi;

		// in pixel
		float width() const;
		float height() const;

		glm::mat4 projMat() const;
	};

	constexpr ViewRectCpo A4PaperViewRect{{0.0f, 0.0f}, {0.297f, 0.21f}, 144.0f};
}
