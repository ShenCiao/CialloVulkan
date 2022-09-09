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
	public:
		std::unique_ptr<ArticulatedLineEngine> m_articulated;
		std::unique_ptr<EquidistantDotEngine> m_equidistantDot;
		vulkan::Buffer m_canvasViewProj;
	public:
		explicit CanvasRenderer(vulkan::Device* device)
		{
			m_articulated = std::make_unique<ArticulatedLineEngine>(device);
			m_equidistantDot = std::make_unique<EquidistantDotEngine>(device);
		}

		void render(vk::CommandBuffer cb, const vulkan::Image* target) const
		{
			vk::ClearColorValue v;
			v.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
			cb.clearColorImage(*target, target->imageLayout(), v,
			                   vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u});

			constexpr vk::MemoryBarrier2 barrier{
				vk::PipelineStageFlagBits2::eAllCommands,
				vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead,
				vk::PipelineStageFlagBits2::eAllCommands,
				vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead,
			};
			cb.pipelineBarrier2({{}, barrier, {}, {}});

			m_equidistantDot->renderDynamic(cb, target);
			m_articulated->renderDynamic(cb, target);
		}
	};
}
