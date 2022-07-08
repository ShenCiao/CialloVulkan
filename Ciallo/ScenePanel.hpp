#pragma once

#include <imgui.h>

#include "Device.hpp"
#include "Image.hpp"
#include "CanvasRenderer.hpp"

namespace ciallo::gui
{
	class ScenePanel
	{
	public:
		void draw() const;
	public:
		void genCanvas(vulkan::Device* d, vk::CommandBuffer cb);
		void genSampler(vk::Device device);
	private:
		std::unique_ptr<vulkan::Image> m_canvas;
		std::unique_ptr<rendering::CanvasRenderer> m_canvasRenderer;
		vk::UniqueSampler m_sampler;
		ImTextureID m_canvasTextureId = nullptr;
	};
	
}
