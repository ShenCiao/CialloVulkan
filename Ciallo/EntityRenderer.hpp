#pragma once
#include "Device.hpp"

namespace ciallo
{
	struct EntityRenderer
	{
	public:
		EntityRenderer() = default;
		EntityRenderer(const EntityRenderer& other) = delete;
		EntityRenderer(EntityRenderer&& other) noexcept = default;
		EntityRenderer& operator=(const EntityRenderer& other) = delete;
		EntityRenderer& operator=(EntityRenderer&& other) noexcept = default;
		virtual ~EntityRenderer() = default;
		
		virtual void render(vk::CommandBuffer cb) = 0;
	};
}
