#pragma once

#include "Buffer.hpp"
#include "Device.hpp"


namespace ciallo
{
	// Supposed to auto bind buffer, write code like this `Buffer<DataCpo>::connect`. But I'm lazy for now.

	struct PolylineBuffer
	{
		vulkan::Buffer buffer;

		static inline entt::observer ob{};
		static void connect(entt::registry& r);
		static void update(entt::registry& r);
		static void updateBuffer(entt::registry& r, entt::entity e, vk::CommandBuffer cb, const vulkan::Device* device);
		static void onConstruct(entt::registry& r, entt::entity e);
	};

	struct WidthPerVertBuffer
	{
		vulkan::Buffer buffer;

		static inline entt::observer ob{};
		static void connect(entt::registry& r);
		static void update(entt::registry& r);
		static void updateBuffer(entt::registry& r, entt::entity e, vk::CommandBuffer cb, const vulkan::Device* device);
		static void onConstruct(entt::registry& r, entt::entity e);
	};

	struct ColorBuffer
	{
		vulkan::Buffer buffer;
		
		static inline entt::observer ob{};
		static void connect(entt::registry& r);
		static void updateBuffer(entt::registry& r, entt::entity e, vk::CommandBuffer cb, const vulkan::Device* device);
		static void onConstruct(entt::registry& r, entt::entity e);
		static void update(entt::registry& r);
	};
}
