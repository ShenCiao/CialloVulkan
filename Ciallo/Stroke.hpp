#pragma once

namespace ciallo
{
	/*
	 * Stroke is the core object of Ciallo. It's constructed by a polyline with width and color.
	 */
	struct StrokeCpo
	{

	};

	struct PolylineCpo
	{
		// Suppose to be a dedicate geom::Polyline class. Temporarily use vector.
		std::vector<geom::Point> polyline{};
	};

	struct WidthVCpo // Width per Vertex
	{
		std::vector<float> width{};
	};

	struct ColorCpo
	{
		glm::vec4 color = {0.0f, 0.0f, 0.0f, 1.0f};
	};
}