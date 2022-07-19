#pragma once
#include <glm/vec4.hpp>
/* Principle brush components are components that supposed to be supported by all brush engines. (Exceptions may occur in the future)
 * All game engines support Ciallo's brush should at least support width per vertex and color of whole strokes.
 */

namespace ciallo::brush
{
	// x from vertex abbreviated as xVComponent. 
	struct WidthVComponent
	{

	};

	struct ColorComponent
	{
		glm::vec4 color;
	};
}
