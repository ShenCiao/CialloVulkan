#pragma once

#include "vk_mem_alloc.h"
#include "vulkanWindow.hpp"

namespace ciallo::vulkan
{
	class Test
	{
	public:
		explicit Test(Window* const w)
			: w(w)
		{
			createGraphicsPipeline();
			createCommandBuffers();
			createSyncObject();
			while (!w->shouldClose())
			{
				glfwPollEvents();
				drawFrame();
			}
			w->device().waitIdle();
		}

	private:
		Window* w;

		void createGraphicsPipeline()
		{
			m_vertexShader = vku::ShaderModule{w->device(), "./shaders/vert.spv"};
			m_fragmentShader = vku::ShaderModule{w->device(), "./shaders/frag.spv"};
			vku::PipelineMaker pm{w->swapchainExtent().width, w->swapchainExtent().height};
			pm.shader(vk::ShaderStageFlagBits::eVertex, m_vertexShader)
			  .shader(vk::ShaderStageFlagBits::eFragment, m_fragmentShader);

			vku::PipelineLayoutMaker plm{};
			m_graphicsPipelineLayout = plm.createUnique(w->device());
			m_graphicsPipeline = pm.createUnique(w->device(), nullptr, *m_graphicsPipelineLayout, w->renderPass());
		}

		void createCommandBuffers()
		{
			m_commandBuffers = w->createCommandBuffers(vk::CommandBufferLevel::ePrimary, w->swapchainImageCount());

			auto recordCommandBuffers = [this](const vk::UniqueCommandBuffer& cb, const vk::Framebuffer fb)
			{
				vk::CommandBufferBeginInfo cbbi{};
				cbbi.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
				cb->begin(cbbi);

				vk::RenderPassBeginInfo rpbi{};
				rpbi.renderPass = w->renderPass();
				rpbi.framebuffer = fb;
				rpbi.renderArea.offset = vk::Offset2D{0, 0};
				rpbi.renderArea.extent = w->swapchainExtent();
				vk::ClearValue clearColor = {std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}};
				rpbi.clearValueCount = 1;
				rpbi.pClearValues = &clearColor;

				cb->beginRenderPass(rpbi, vk::SubpassContents::eInline);
				{
					cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_graphicsPipeline);
					cb->draw(3, 1, 0, 0);
				}
				cb->endRenderPass();
				cb->end();
			};

			auto framebuffers = w->framebuffers();

			for (auto [cb, fb] : views::zip(m_commandBuffers, framebuffers))
			{
				recordCommandBuffers(cb, fb);
			}
		}

		void createSyncObject()
		{
			m_imageAvailable = w->device().createSemaphoreUnique({});
			m_renderFinished = w->device().createSemaphoreUnique({});
			vk::FenceCreateInfo info{
				vk::FenceCreateFlagBits::eSignaled
			};
			m_inFlight = w->device().createFenceUnique(info);
		}

		void drawFrame()
		{
			vk::Result _;
			_ = w->device().waitForFences(1, &*m_inFlight, VK_TRUE, std::numeric_limits<uint64_t>::max());
			_ = w->device().resetFences(1, &*m_inFlight);

			uint32_t index;
			try
			{
				index = w->device().acquireNextImageKHR(w->swapchain(), UINT64_MAX, *m_imageAvailable,
				                                                 VK_NULL_HANDLE).value;
			}
			catch (vk::OutOfDateKHRError& err)
			{
				w->recreateOnWindowResize();
				return;
			}
			catch (vk::SystemError& err)
			{
				throw std::runtime_error("failed to acquire swap chain image!");
			}
			std::vector<vk::PipelineStageFlags> waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
			vk::SubmitInfo si{
				*m_imageAvailable,
				waitStages,
				*m_commandBuffers[index],
				*m_renderFinished
			};
			w->queue().submit(si, *m_inFlight);

			auto swapchain = w->swapchain();
			vk::PresentInfoKHR pi{
				*m_renderFinished,
				swapchain,
				index,
				{}
			};
			_ = w->queue().presentKHR(pi);
		}

		vku::ShaderModule m_fragmentShader;
		vku::ShaderModule m_vertexShader;
		vk::UniquePipeline m_graphicsPipeline;
		vk::UniquePipelineLayout m_graphicsPipelineLayout;
		std::vector<vk::UniqueCommandBuffer> m_commandBuffers;
		vk::UniqueSemaphore m_imageAvailable;
		vk::UniqueSemaphore m_renderFinished;
		vk::UniqueFence m_inFlight;
	};
}
