#pragma once

namespace ciallo
{
	/*
	 * Stroke is the core object of Ciallo. It's constructed by a polyline and it's thickness.
	 * Ciallo takes thickness as a part of stroke itself instead of brush or rendering. It's a controversial decision but I get good reason for that.
	 */

	struct StrokeCpo
	{
		std::vector<geom::Point> position{}; // a dedicated class geom::Polyline is needed here
		std::vector<float> thickness{};

		entt::entity brush{entt::null};

		StrokeCpo() = default;
	};
}
