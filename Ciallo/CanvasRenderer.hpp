#pragma once
#include "Image.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace ciallo::rendering
{
	class CanvasRenderer
	{
		std::unique_ptr<vulkan::Buffer> m_vertexBuffer;
		vulkan::Image* m_target = nullptr;
	public:
		CanvasRenderer() = default;

		void setTarget(vulkan::Image* target)
		{
			m_target = target;
		}

		void render(vk::CommandBuffer cb)
		{
			vk::RenderingAttachmentInfo colorAttchment{m_target->imageView(), m_target->imageLayout()};
			colorAttchment.setLoadOp(vk::AttachmentLoadOp::eLoad);
			colorAttchment.setStoreOp(vk::AttachmentStoreOp::eStore);

			vk::RenderingInfo renderingInfo{
				{},
				vk::Rect2D{{0, 0}, m_target->extent()},
				0,
				0,
				colorAttchment
			};
			cb.beginRendering(renderingInfo);

		}

		void genVertexBuffer(VmaAllocator allocator)
		{
			std::vector<glm::vec3> vert;
			vert.reserve(1024);
			for (int i : views::iota(0, 1024))
			{
				float x = i * 2.0f * glm::pi<float>() / 1024.0f;
				float y = glm::sin(x);

				vert.emplace_back(x, y, 1.0f);
			}

			VmaAllocationCreateInfo info{VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO};
			m_vertexBuffer = std::make_unique<vulkan::Buffer>(allocator, info, sizeof(glm::vec3) * vert.size(),
			                                                  vk::BufferUsageFlagBits::eVertexBuffer);
			m_vertexBuffer->uploadLocal(vert.data(), sizeof(glm::vec3) * vert.size());
		}
	};
}
