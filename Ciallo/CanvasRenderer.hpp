#pragma once
#include "Image.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include "ArticulatedBrush.hpp"
#include "Device.hpp"

namespace ciallo::rendering
{
	class CanvasRenderer
	{
		std::unique_ptr<vulkan::Buffer> m_vertexBuffer;
		vulkan::Image* m_target = nullptr;
		std::unique_ptr<rendering::Articulated> m_articulated;
		std::unique_ptr<vulkan::Buffer> m_uniformVert;
		std::unique_ptr<vulkan::Buffer> m_uniformFrag;
	public:
		explicit CanvasRenderer(vulkan::Device* device)
		{
			m_articulated = std::make_unique<rendering::Articulated>(*device);
			genVertexBuffer(*device);
			genUniformBuffer(*device);
			m_articulated->genDescriptorSet(device->descriptorPool(), *m_uniformVert, *m_uniformFrag);
		}

		void genUniformBuffer(VmaAllocator allocator)
		{
			VmaAllocationCreateInfo info{VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO};
			m_uniformVert = std::make_unique<vulkan::Buffer>(allocator, info, sizeof(float), vk::BufferUsageFlagBits::eUniformBuffer);
			m_uniformFrag = std::make_unique<vulkan::Buffer>(allocator, info, sizeof(glm::vec4), vk::BufferUsageFlagBits::eUniformBuffer);
			float width = 0.1f;
			m_uniformVert->uploadLocal(&width, sizeof(float));
			glm::vec4 color = {0.1f, 0.1f, 0.1f, 1.0f};
			m_uniformFrag->uploadLocal(&color, sizeof(glm::vec4));
		}

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
			cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_articulated->m_pipeline);
			vk::Viewport fullViewport{0, 0, static_cast<float>(m_target->width()), static_cast<float>(m_target->height())};
			cb.setViewport(0, fullViewport);
			vk::Rect2D fullScissor{{0, 0}, m_target->extent()};
			cb.setScissor(0, fullScissor);
			cb.bindVertexBuffers(0, m_vertexBuffer->buffer(), {0u});
			cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_articulated->m_pipelineLayout, 0, 1, &m_articulated->m_descriptorSet, 0, nullptr);
			cb.draw(1024, 1, 0, 0);
			cb.endRendering();
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
