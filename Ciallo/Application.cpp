#include "pch.hpp"
#include "Application.hpp"
#include "MainPassRenderer.hpp"
#include "ScenePanel.hpp"

void ciallo::Application::run() const
{
	auto w = std::make_unique<vulkan::Window>(1000, 1000, "hi");
	auto inst = std::make_shared<vulkan::Instance>();
	inst->addInstanceExtensions(w->getRequiredInstanceExtensions());
	inst->create();
	w->setInstance(inst);
	w->initResources();

	vk::UniqueSemaphore presentImageAvailableSemaphore = w->device().createSemaphoreUnique({});
	vulkan::MainPassRenderer mainPassRenderer(w.get());
	mainPassRenderer.init();
	w->show();
	vk::UniqueCommandBuffer cb = std::move(w->createCommandBuffers(vk::CommandBufferLevel::ePrimary, 1)[0]);

	VmaAllocatorCreateInfo allocatorCreateInfo{};
	allocatorCreateInfo.instance = w->instance();
	allocatorCreateInfo.device = w->device();
	allocatorCreateInfo.physicalDevice = w->physicalDevice();
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;

	VmaAllocator allocator;
	vmaCreateAllocator(&allocatorCreateInfo, &allocator);

	gui::ScenePanel sp;

	w->executeImmediately([&](vk::CommandBuffer cb)
	{
		sp.createSampler(w->device());
		sp.createCanvas(allocator, cb);
	});

	w->device().waitIdle();

	while (!w->shouldClose())
	{
		w->pollEvents();
		vk::Result _;
		_ = w->device().waitForFences(mainPassRenderer.renderingCompleteFence(), VK_TRUE,
		                              std::numeric_limits<uint64_t>::max());

		uint32_t index;
		try
		{
			index = w->device().acquireNextImageKHR(w->swapchain(), UINT64_MAX, *presentImageAvailableSemaphore,
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
		w->device().resetFences(mainPassRenderer.renderingCompleteFence());

		vk::CommandBufferBeginInfo cbbi{vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr};
		cb->begin(cbbi);
		ImGui_ImplVulkan_NewFrame();
		w->imguiNewFrame();
		ImGui::NewFrame();
		// -----------------------------------------------------------------------------
		static bool show_demo_window = true;
		if (show_demo_window)
		{
			ImGui::ShowDemoWindow(&show_demo_window);
		}
		sp.draw();
		// -----------------------------------------------------------------------------
		ImGui::EndFrame();
		ImGui::Render();
		ImDrawData* main_draw_data = ImGui::GetDrawData();
		mainPassRenderer.render(*cb, index, main_draw_data);
		cb->end();
		std::vector<vk::PipelineStageFlags> waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};

		std::vector<vk::Semaphore> submitSignalSemaphores = {mainPassRenderer.renderingCompleteSemaphore()};
		vk::SubmitInfo si{
			*presentImageAvailableSemaphore,
			waitStages,
			*cb,
			submitSignalSemaphores
		};
		w->queue().submit(si, mainPassRenderer.renderingCompleteFence());

		auto swapchain = w->swapchain();
		vk::PresentInfoKHR pi{
			submitSignalSemaphores,
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
