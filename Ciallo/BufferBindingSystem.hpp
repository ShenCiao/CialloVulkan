#pragma once
#include "StrokeBufferObjects.hpp"

namespace ciallo
{
	struct BufferBindingSystem
	{
		static void connect(entt::registry& r)
		{
			PolylineBuffer::connect(r);
			WidthPerVertBuffer::connect(r);
			ColorBuffer::connect(r);
		}

		static void update(entt::registry& r)
		{
			PolylineBuffer::update(r);
			WidthPerVertBuffer::update(r);
			ColorBuffer::update(r);
		}
	};
}
