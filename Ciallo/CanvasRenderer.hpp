#pragma once
#include "Image.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include "ArticulatedProgress.hpp"
#include "Device.hpp"

namespace ciallo::rendering
{
	class CanvasRenderer
	{
		std::unique_ptr<vulkan::Buffer> m_vertexBuffer;
		std::unique_ptr<vulkan::Buffer> m_uniformVert;
		std::unique_ptr<vulkan::Buffer> m_uniformFrag;
		std::unique_ptr<rendering::ArticulatedProgress> m_articulated;
	public:
		explicit CanvasRenderer(vulkan::Device* device)
		{
			m_articulated = std::make_unique<rendering::ArticulatedProgress>(device);
		}


		void render(vk::CommandBuffer cb, const vulkan::Image* target)
		{
			m_articulated->renderDynamic(cb, target);
		}
	};
}
