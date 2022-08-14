#pragma once
#include "CommonSceneObjectComponents.hpp"

namespace ciallo::scene
{
	/*
	 * Stroke is the core object of Ciallo. It's constructed by a polyline and information of how to render the polyline. 
	 */
	struct PolylineCpo
	{
		// Suppose to be a dedicate geom::Polyline class. Temporarily use vector.
		std::vector<geom::Point> polyline; 
	};

	struct BrushCpo
	{
		entt::handle brush;
	};

	struct WidthCpo
	{
		
	};
}