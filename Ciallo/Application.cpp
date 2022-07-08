#include "pch.hpp"
#include "Application.hpp"

#include <implot.h>

#include "Device.hpp"
#include "MainPassRenderer.hpp"
#include "ScenePanel.hpp"
#include "ShaderModule.hpp"
#include "ArticulatedBrush.hpp"

void ciallo::Application::run() const
{
	auto w = std::make_unique<vulkan::Window>(1000, 1000, "hi");
	vulkan::Instance::addExtensions(vulkan::Window::getRequiredInstanceExtensions());
	auto inst = std::make_shared<vulkan::Instance>();
	int physicalDeviceIndex = vulkan::Device::pickPhysicalDevice(*inst);
	auto d = std::make_shared<vulkan::Device>(*inst, physicalDeviceIndex);


	w->setInstance(inst);
	w->setDevice(d);
	w->initResources();

	vk::UniqueSemaphore presentImageAvailableSemaphore = d->device().createSemaphoreUnique({});
	vulkan::MainPassRenderer mainPassRenderer(w.get());
	w->show();
	vk::CommandBuffer cb = d->createCommandBuffer();

	gui::ScenePanel sp;

	d->executeImmediately([&](vk::CommandBuffer c)
	{
		sp.genSampler(*d);
		sp.genCanvas(d.get(), c);
	});

	d->device().waitIdle();

	while (!w->shouldClose())
	{
		w->pollEvents();
		vk::Result _;
		_ = d->device().waitForFences(mainPassRenderer.renderingCompleteFence(), VK_TRUE,
		                              std::numeric_limits<uint64_t>::max());

		uint32_t index;
		try
		{
			index = d->device().acquireNextImageKHR(w->swapchain(), UINT64_MAX, *presentImageAvailableSemaphore,
			                                        VK_NULL_HANDLE).value;
		}
		catch (vk::OutOfDateKHRError&)
		{
			w->onWindowResize();
			mainPassRenderer.genFramebuffers();
			continue;
		}
		catch (vk::SystemError&)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
		d->device().resetFences(mainPassRenderer.renderingCompleteFence());

		vk::CommandBufferBeginInfo cbbi{vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr};
		cb.begin(cbbi);
		ImGui_ImplVulkan_NewFrame();
		w->imguiNewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		// -----------------------------------------------------------------------------
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::MenuItem("LoadShader"))
			{
				auto test = vulkan::ShaderModule(*d, vk::ShaderStageFlagBits::eVertex, "./shaders/articulated.vert.spv");
				spdlog::info("successfully loaded");
			}
			ImGui::EndMainMenuBar();
		}

		static bool show_demo_window = true;
		if (show_demo_window)
		{
			ImGui::ShowDemoWindow(&show_demo_window);
		}
		static bool show_demo = true;
		if (show_demo)
		{
			ImPlot::ShowDemoWindow(&show_demo);
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
		d->queue().submit(si, mainPassRenderer.renderingCompleteFence());

		auto swapchain = w->swapchain();
		vk::PresentInfoKHR pi{
			submitSignalSemaphores,
			swapchain,
			index,
			{}
		};

		try
		{
			_ = d->queue().presentKHR(pi);
		}
		catch (vk::OutOfDateKHRError&)
		{
			w->onWindowResize();
			mainPassRenderer.genFramebuffers();
			continue;
		}
		catch (vk::SystemError&)
		{
			throw std::runtime_error("Failed to present!");
		}
	}

	d->device().waitIdle();
}

void ciallo::Application::loadSettings()
{
	m_mainWindowWidth = 400;
	m_mainWindowHeight = 400;
}
