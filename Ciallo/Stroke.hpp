#pragma once

namespace ciallo
{
	/*
	 * Stroke is the core object of Ciallo. It's constructed by a polyline and it's thickness.
	 * Ciallo takes thickness as a part of stroke itself instead of brush or rendering. It's a controversial decision but I get good reason for that.
	 */

	struct StrokeTag
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

	struct BrushCpo
	{
		entt::entity brush{entt::null};
	};
}
