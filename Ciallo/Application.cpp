#include "pch.hpp"
#include "Application.hpp"

#include <implot.h>

#include "Device.hpp"
#include "MainPassRenderer.hpp"
#include "ScenePanel.hpp"
#include "BrushPool.hpp"

void ciallo::Application::run()
{
	// --- Move these to somewhere else someday ---------------------------------
	auto window = std::make_unique<vulkan::Window>(1000, 1000, "hi");
	vulkan::Instance::addExtensions(vulkan::Window::getRequiredInstanceExtensions());
	m_instance = std::make_shared<vulkan::Instance>();
	window->setInstance(*m_instance);
	vk::SurfaceKHR surface = window->genSurface();
	
	vk::PhysicalDevice physicalDevice = vulkan::Device::pickPhysicalDevice(*m_instance, surface);
	uint32_t queueIndex = vulkan::Device::findRequiredQueueFamily(physicalDevice, surface);
	m_device = std::make_shared<vulkan::Device>(*m_instance, physicalDevice, queueIndex);
	window->setDevice(*m_device);
	window->setPhysicalDevice(m_device->physicalDevice());
	window->initSwapchain();

	auto brushPool = std::make_unique<editor::BrushPool>();
	brushPool->loadPresetBrushes(m_device.get());
	// -----------------------------------------------------------------------------

	vk::UniqueSemaphore presentImageAvailableSemaphore = m_device->device().createSemaphoreUnique({});
	vulkan::MainPassRenderer mainPassRenderer(window.get(), m_device.get());
	window->show();
	vk::CommandBuffer cb = m_device->createCommandBuffer();

	gui::ScenePanel sp;

	m_device->executeImmediately([&](vk::CommandBuffer tmpcb)
	{
		sp.genSampler(*m_device);
		sp.genCanvas(m_device.get(), tmpcb);
	});

	m_device->device().waitIdle();

	while (!window->shouldClose())
	{
		window->pollEvents();
		vk::Result _;
		_ = m_device->device().waitForFences(mainPassRenderer.renderingCompleteFence(), VK_TRUE,
		                                   std::numeric_limits<uint64_t>::max());

		uint32_t index;
		try
		{
			index = m_device->device().acquireNextImageKHR(window->swapchain(), UINT64_MAX,
			                                             *presentImageAvailableSemaphore,
			                                             VK_NULL_HANDLE).value;
		}
		catch (vk::OutOfDateKHRError&)
		{
			window->onWindowResize();
			mainPassRenderer.genFramebuffers();
			continue;
		}
		catch (vk::SystemError&)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
		m_device->device().resetFences(mainPassRenderer.renderingCompleteFence());

		vk::CommandBufferBeginInfo cbbi{vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr};
		cb.begin(cbbi);
		ImGui_ImplVulkan_NewFrame();
		window->imguiNewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		// -----------------------------------------------------------------------------
		if (ImGui::BeginMainMenuBar())
		{
			ImGui::EndMainMenuBar();
		}

		static bool show_demo_window = true;
		if (show_demo_window)
		{
			ImGui::ShowDemoWindow(&show_demo_window);
		}
		static bool show_demo_plot = true;
		if (show_demo_plot)
		{
			ImPlot::ShowDemoWindow(&show_demo_plot);
		}

		sp.draw();
		// -----------------------------------------------------------------------------
		ImGui::EndFrame();
		ImGui::Render();
		ImDrawData* main_draw_data = ImGui::GetDrawData();
		mainPassRenderer.render(cb, index, main_draw_data);
		cb.end();
		std::vector<vk::PipelineStageFlags> waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};

		std::vector<vk::Semaphore> submitSignalSemaphores = {mainPassRenderer.renderingCompleteSemaphore()};
		vk::SubmitInfo si{
			*presentImageAvailableSemaphore,
			waitStages,
			cb,
			submitSignalSemaphores
		};
		m_device->queue().submit(si, mainPassRenderer.renderingCompleteFence());

		auto swapchain = window->swapchain();
		vk::PresentInfoKHR pi{
			submitSignalSemaphores,
			swapchain,
			index,
			{}
		};

		try
		{
			_ = m_device->queue().presentKHR(pi);
		}
		catch (vk::OutOfDateKHRError&)
		{
			window->onWindowResize();
			mainPassRenderer.genFramebuffers();
			continue;
		}
		catch (vk::SystemError&)
		{
			throw std::runtime_error("Failed to present!");
		}
	}

	m_device->device().waitIdle();
}

void ciallo::Application::loadSettings()
{
}
