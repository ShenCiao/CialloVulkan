#pragma once

namespace ciallo::rendering
{
	class Renderer
	{
	public:
		Renderer() = default;
		virtual ~Renderer() = 0;
		virtual void render(vk::CommandBuffer cb, entt::handle object) = 0;
	};
}
