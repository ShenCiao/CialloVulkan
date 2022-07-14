#pragma once
#include "ArticulatedProgress.hpp"
#include "Device.hpp"
#include "EquidistantDot.hpp"
#include "Image.hpp"

namespace ciallo::rendering
{
	class CanvasRenderer
	{
		std::unique_ptr<vulkan::Buffer> m_vertexBuffer;
		std::unique_ptr<vulkan::Buffer> m_uniformVert;
		std::unique_ptr<vulkan::Buffer> m_uniformFrag;
		std::unique_ptr<rendering::ArticulatedProgress> m_articulated;
		std::unique_ptr<rendering::EquidistantDot> m_equidistantDot;
		
	public:
		explicit CanvasRenderer(vulkan::Device* device)
		{
			m_articulated = std::make_unique<rendering::ArticulatedProgress>(device);
			m_equidistantDot = std::make_unique<rendering::EquidistantDot>(device);
		}


		void render(vk::CommandBuffer cb, const vulkan::Image* target)
		{
			m_equidistantDot->renderDynamic(cb, target);
		}
	};
}
