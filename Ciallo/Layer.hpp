#pragma once

namespace ciallo
{
	enum class BlendMode
	{
		Normal,
		Add,
		Subtract,
	};

	struct LayerCpo
	{
		float alpha = 1.0f;
		BlendMode blend = BlendMode::Normal;
	};
}
