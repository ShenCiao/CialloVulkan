#pragma once

namespace ciallo
{
	/*
	 * Stroke is the core object of Ciallo. It's constructed by a polyline and it's thickness.
	 * Ciallo takes thickness as a part of geometry. It's a controversial decision but I get good reason for that.
	 */

	struct StrokeCpo
	{

	};

	struct PolylineCpo
	{
		// Suppose to be a dedicate geom::Polyline class. Temporarily use vector.
		std::vector<geom::Point> polyline{};
	};

	struct ThicknessPerVertCpo // Width per vertex
	{
		std::vector<float> thickness{};
	};

	struct ColorCpo
	{
		glm::vec4 color = {0.0f, 0.0f, 0.0f, 1.0f};
	};
}