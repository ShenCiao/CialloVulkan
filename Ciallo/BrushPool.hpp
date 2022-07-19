#pragma once
#include "Image.hpp"
#include "Device.hpp"
#include "imgui.h"

namespace ciallo
{
	namespace brush
	{
		enum class EngineFlags
		{
			None,
			EquidistantDot,
			ArticulatedLine
		};

		struct DemoComponent
		{
			vulkan::Image image;
			ImTextureID id = nullptr;
		};

		struct EngineComponent
		{
			EngineFlags engineType;
		};

		struct ArticulatedLineSettings
		{
		};
	}

	class BrushPool
	{
		entt::registry m_registry;
		std::vector<entt::entity> m_brushes = {};

	public:
		BrushPool() = default;


		void loadPresetBrushes(vulkan::Device* device)
		{
			entt::entity brush = m_registry.create();
			m_registry.emplace<brush::EngineFlags>(brush, brush::EngineFlags::ArticulatedLine);
			m_registry.emplace<brush::ArticulatedLineSettings>(brush);

			for (int i : views::iota(0, 1000))
			{
				m_brushes.push_back(m_registry.create());
			}

			VmaAllocationCreateInfo allocationCreateInfo{{}, VMA_MEMORY_USAGE_AUTO};
			brush::DemoComponent demo{
				vulkan::Image(*device, allocationCreateInfo, 400, 100,
				              vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment)
			};

			m_registry.insert<brush::DemoComponent>(m_brushes.begin(), m_brushes.end(), demo);
		}
	};
}
