#pragma once

namespace ciallo
{
	class ObjectRenderer
	{
	public:
		ObjectRenderer() = default;
		virtual ~ObjectRenderer() = 0;
		virtual void render(vk::CommandBuffer cb, entt::handle object) = 0;
	};
}
