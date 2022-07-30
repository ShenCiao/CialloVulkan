#pragma once
#include <imgui.h>
#include <glm/vec4.hpp>

#include "Image.hpp"
/* Principle brush components are components that supposed to be supported by all brush engines. (Few exceptions may occur in the future)
 * All game engines support Ciallo's brush should at least support width per vertices and color of whole strokes.
 */

namespace ciallo::editor
{
	// Value x should fetch from vertices abbreviated as xVCpo. 
	struct WidthVCpo
	{
	};

	struct ColorCpo
	{
		glm::vec4 color;
	};

	enum class EngineTypeFlags
	{
		None,
		EquidistantDot,
		ArticulatedLine
	};

	struct DemoCpo
	{
		vulkan::Image image;
		ImTextureID guiId = nullptr;
	};

	struct EngineTagCpo
	{
		EngineTypeFlags engineType;
	};
}
