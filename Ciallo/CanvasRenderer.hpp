#pragma once
#include "ArticulatedLine.hpp"
#include "Device.hpp"
#include "EquidistantDot.hpp"
#include "Image.hpp"
#include <glm/glm.hpp>

namespace ciallo::rendering
{
	class CanvasRenderer
	{
		std::unique_ptr<vulkan::Buffer> m_vertexBuffer;
		std::unique_ptr<vulkan::Buffer> m_uniformVert;
		std::unique_ptr<vulkan::Buffer> m_uniformFrag;
		std::unique_ptr<rendering::ArticulatedLine> m_articulated;
		std::unique_ptr<rendering::EquidistantDot> m_equidistantDot;
		vulkan::Buffer m_canvasViewProj;
	public:
		explicit CanvasRenderer(vulkan::Device* device)
		{
			m_articulated = std::make_unique<rendering::ArticulatedLine>(device);
			m_equidistantDot = std::make_unique<rendering::EquidistantDot>(device);
		}


		void render(vk::CommandBuffer cb, const vulkan::Image* target)
		{
			m_articulated->renderDynamic(cb, target);
			m_equidistantDot->renderDynamic(cb, target);
		}
	};
}
