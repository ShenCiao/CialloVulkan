#include "pch.hpp"
#include "Application.hpp"

#include <implot.h>

#include "Device.hpp"
#include "MainPassRenderer.hpp"
#include "ScenePanel.hpp"
#include "BrushPool.hpp"

void ciallo::Application::run() const
{
	auto win = std::make_unique<vulkan::Window>(1000, 1000, "hi");
	vulkan::Instance::addExtensions(vulkan::Window::getRequiredInstanceExtensions());
	auto inst = std::make_shared<vulkan::Instance>();
	int physicalDeviceIndex = vulkan::Device::pickPhysicalDevice(*inst);
	auto device = std::make_shared<vulkan::Device>(*inst, physicalDeviceIndex);
	auto brushPool = std::make_unique<BrushPool>();
	brushPool->loadPresetBrushes(device.get());

	win->setInstance(inst);
	win->setDevice(device);
	win->initResources();

	vk::UniqueSemaphore presentImageAvailableSemaphore = device->device().createSemaphoreUnique({});
	vulkan::MainPassRenderer mainPassRenderer(win.get());
	win->show();
	vk::CommandBuffer cb = device->createCommandBuffer();

	gui::ScenePanel sp;

	device->executeImmediately([&](vk::CommandBuffer tmpcb)
	{
		sp.genSampler(*device);
		sp.genCanvas(device.get(), tmpcb);
	});

	device->device().waitIdle();

	while (!win->shouldClose())
	{
		win->pollEvents();
		vk::Result _;
		_ = device->device().waitForFences(mainPassRenderer.renderingCompleteFence(), VK_TRUE,
		                                   std::numeric_limits<uint64_t>::max());

		uint32_t index;
		try
		{
			index = device->device().acquireNextImageKHR(win->swapchain(), UINT64_MAX,
			                                             *presentImageAvailableSemaphore,
			                                             VK_NULL_HANDLE).value;
		}
		catch (vk::OutOfDateKHRError&)
		{
			win->onWindowResize();
			mainPassRenderer.genFramebuffers();
			continue;
		}
		catch (vk::SystemError&)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
		device->device().resetFences(mainPassRenderer.renderingCompleteFence());

		vk::CommandBufferBeginInfo cbbi{vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr};
		cb.begin(cbbi);
		ImGui_ImplVulkan_NewFrame();
		win->imguiNewFrame();
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
		device->queue().submit(si, mainPassRenderer.renderingCompleteFence());

		auto swapchain = win->swapchain();
		vk::PresentInfoKHR pi{
			submitSignalSemaphores,
			swapchain,
			index,
			{}
		};

		try
		{
			_ = device->queue().presentKHR(pi);
		}
		catch (vk::OutOfDateKHRError&)
		{
			win->onWindowResize();
			mainPassRenderer.genFramebuffers();
			continue;
		}
		catch (vk::SystemError&)
		{
			throw std::runtime_error("Failed to present!");
		}
	}

	device->device().waitIdle();
}

void ciallo::Application::loadSettings()
{
	m_mainWindowWidth = 400;
	m_mainWindowHeight = 400;
}
