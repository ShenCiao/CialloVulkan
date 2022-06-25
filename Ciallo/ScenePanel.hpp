#pragma once

#include <imgui.h>
#include "Image.hpp"

namespace ciallo::gui
{
	class ScenePanel
	{
	public:
		void draw() const;
	public:
		void genCanvas(VmaAllocator allocator, vk::CommandBuffer cb);
		void genSampler(vk::Device device);
	private:
		std::unique_ptr<vulkan::Image> m_canvas;
		vk::UniqueSampler m_sampler;
		ImTextureID m_canvasTextureId = nullptr;
	};
	
}
