#include "pch.hpp"
#include "Application.hpp"
#include "MainPassRenderer.hpp"

void ciallo::Application::run()
{
	auto w = std::make_unique<vulkan::Window>(1000, 1000, "hi");
	auto inst = std::make_shared<vulkan::Instance>();
	inst->addInstanceExtensions(w->getRequiredInstanceExtensions());
	inst->create();
	w->setInstance(inst);
	w->initResources();

	vk::UniqueSemaphore imageAvailableSemaphore = w->device().createSemaphoreUnique({});
	vulkan::MainPassRenderer mainPassRenderer(w.get());
	mainPassRenderer.init();
	w->show();
	vk::UniqueCommandBuffer cb = std::move(w->createCommandBuffers(vk::CommandBufferLevel::ePrimary, 1)[0]);
	while (!w->shouldClose())
	{
		w->pollEvents();
		vk::Result _;
		_ = w->device().waitForFences(*mainPassRenderer.m_renderingCompleteFence, VK_TRUE,
		                              std::numeric_limits<uint64_t>::max());

		uint32_t index;
		try
		{
			index = w->device().acquireNextImageKHR(w->swapchain(), UINT64_MAX, *imageAvailableSemaphore,
			                                        VK_NULL_HANDLE).value;
		}
		catch (vk::OutOfDateKHRError&)
		{
			w->onWindowResize();
			mainPassRenderer.createFramebuffers();
			continue;
		}
		catch (vk::SystemError&)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
		w->device().resetFences(*mainPassRenderer.m_renderingCompleteFence);

		vk::CommandBufferBeginInfo cbbi{vk::CommandBufferUsageFlagBits::eSimultaneousUse, nullptr};
		cb->begin(cbbi);
		// -----------------------------------------------------------------------------
		ImGui_ImplVulkan_NewFrame();
		w->imguiNewFrame();
		ImGui::NewFrame();

		static bool show_demo_window = true;
		if (show_demo_window)
		{
			ImGui::ShowDemoWindow(&show_demo_window);
		}
		ImGui::Render();
		ImDrawData* main_draw_data = ImGui::GetDrawData();
		ImGui::EndFrame();
		// -----------------------------------------------------------------------------
		mainPassRenderer.render(*cb, index, main_draw_data);
		cb->end();
		std::vector<vk::PipelineStageFlags> waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::SubmitInfo si{
			*imageAvailableSemaphore,
			waitStages,
			*cb,
			*mainPassRenderer.m_renderingCompleteSemaphore
		};
		w->queue().submit(si, *mainPassRenderer.m_renderingCompleteFence);
		
		auto swapchain = w->swapchain();
		vk::PresentInfoKHR pi{
			*mainPassRenderer.m_renderingCompleteSemaphore,
			swapchain,
			index,
			{}
		};

		try
		{
			_ = w->queue().presentKHR(pi);
		}
		catch (vk::OutOfDateKHRError&)
		{
			w->onWindowResize();
			mainPassRenderer.createFramebuffers();
			continue;
		}
		catch (vk::SystemError&)
		{
			throw std::runtime_error("Failed to present!");
		}
	}

	w->device().waitIdle();
}

void ciallo::Application::loadSettings()
{
	m_mainWindowWidth = 400;
	m_mainWindowHeight = 400;
}
