#include "pch.hpp"
#include "Drawing.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ciallo
{
	float ViewRectCpo::width() const
	{
		return (max - min).x * dpi / 0.0254f;
	}

	float ViewRectCpo::height() const
	{
		return (max - min).y * dpi / 0.0254f;
	}

	glm::mat4 ViewRectCpo::projMat() const
	{
		return glm::ortho(min.x, max.x, max.y, min.y);
	}
}
