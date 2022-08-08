#pragma once
#include <imgui.h>
#include <glm/vec4.hpp>

#include "Image.hpp"

namespace ciallo::rendering
{
	enum class EngineType
	{
		None,
		EquidistantDot,
		ArticulatedLine
	};
}

namespace ciallo::editor::brush
{
	/* Brush is used for generating strokes. It store settings and ...
	 * From regular users' perspective, polyline + brush = visible stroke. But actually it's polyline + other stroke components + renderer = visible stroke;
	 * Engine settings are hidden inside brush settings. brush is responsible for generating components in stroke entities.
	 * Game engines support Ciallo's brush should at least support width per vertex and color per stroke.
	 */

	enum class AttributeSource
	{
		// Visual attributes can be stored either per vertex or per stroke level.
		None,
		Vertex,
		Stroke,
		InnerShader = None,
	};

	struct WidthSettingsCpo
	{
		AttributeSource source = AttributeSource::None;
		float width = 0.0f;
		
	};

	struct ColorSettingsCpo
	{
		AttributeSource source = AttributeSource::None;
		glm::vec4 color;
	};

	struct DemoCpo
	{
		vulkan::Image image;
		ImTextureID guiId = nullptr;
	};

	struct EngineTypeCpo
	{
		rendering::EngineType engineType;
	};
}
