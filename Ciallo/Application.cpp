#include "pch.hpp"
#include "Application.hpp"

#include <implot.h>

#include "Device.hpp"
#include "MainPassRenderer.hpp"
#include "CanvasPanel.hpp"
#include "CanvasRenderer.hpp"
#include "Drawing.hpp"
#include "Image.hpp"
#include "Project.hpp"

void ciallo::Application::run()
{
	// --- Move these to somewhere else someday ---------------------------------
	auto window = std::make_unique<vulkan::Window>(1024u, 1024u, "Ciallo  - Laboratory Version");
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

	vulkan::MainPassRenderer mainPassRenderer(window.get(), m_device.get());
	// -----------------------------------------------------------------------------
	Project project = createDefaultProject();
	entt::registry& registry = project.registry();
	// -----------------------------------------------------------------------------

	vk::UniqueSemaphore presentImageAvailableSemaphore = m_device->device().createSemaphoreUnique({});
	window->show();
	vk::CommandBuffer cb = m_device->createCommandBuffer();

	auto canvasRenderer = std::make_unique<rendering::CanvasRenderer>(m_device.get());

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
		// --start imgui recording------------------------------------------------------
		entt::entity tempe = registry.view<CanvasPanelCpo>()[0];
		canvasRenderer->render(cb, &registry.get<VulkanImageCpo>(registry.get<CanvasPanelCpo>(tempe).drawing).image);
		CanvasPanelDrawer::update(registry);
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
		// -----------------------------------------------------------------------------
		static float t = 0.0f;
		t += 0.004f;
		t = glm::mod(t, 3.0f);
		float a = glm::mod(t, 1.0f);
		a = glm::smoothstep(0.0f, 1.0f, a);
		ImGui::Begin("Hexagram Control", nullptr);
		ImGui::Text("Upward Triangle drawn by Articulated Line Engine (NDC space)");
		auto& al = canvasRenderer->m_articulated->vertices;
		glm::vec4 red = {1.0f, 0.0f, 0.0f, 1.0f};
		glm::vec4 green = {0.0f, 1.0f, 0.0f, 1.0f};
		glm::vec4 blue = {0.0f, 0.0f, 1.0f, 1.0f};

		for (int i : views::iota(0, 3))
		{
			ImGui::Text("Vertex #%d", i);
			ImGui::DragFloat2(fmt::format("Position##a{}", i).c_str(), reinterpret_cast<float*>(&al.at(i).pos), 0.01f,
			                  -1.0f, 1.0f);
			ImGui::ColorEdit4(fmt::format("Color##a{}", i).c_str(), reinterpret_cast<float*>(&al.at(i).color));
			ImGui::DragFloat(fmt::format("Width##a{}", i).c_str(), &al.at(i).width, 0.001f, 0.0f, 0.1f);
		}
		al[3] = al[0];

		ImGui::Separator();

		ImGui::Text("Downward Triangle drawn by Equidistant Dot Engine (NDC space)");
		ImGui::Text("Spacing control distance between dots");
		ImGui::DragFloat("Spacing", &canvasRenderer->m_equidistantDot->spacing, 0.001f, 0.0001f, 1.0f);
		auto& ed = canvasRenderer->m_equidistantDot->vertices;

		for (int i : views::iota(0, 3))
		{
			ImGui::Text("Vertex #%d", i);
			ImGui::DragFloat2(fmt::format("Position##b{}", i).c_str(), reinterpret_cast<float*>(&ed.at(i).pos), 0.01f,
			                  -1.0f, 1.0f);
			ImGui::ColorEdit4(fmt::format("Color##b{}", i).c_str(), reinterpret_cast<float*>(&ed.at(i).color));
			ImGui::DragFloat(fmt::format("Width##b{}", i).c_str(), &ed.at(i).width, 0.001f, 0.0f, 0.1f);
		}
		ed[3] = ed[0];
		ImGui::End();
		// -----------------------------------------------------------------------------
		ImGui::EndFrame();
		ImGui::Render();
		ImDrawData* main_draw_data = ImGui::GetDrawData();
		mainPassRenderer.render(cb, index, main_draw_data);
		cb.end();
		std::vector<vk::PipelineStageFlags> waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};

		std::vector<vk::Semaphore> signalAfterRenderingSemaphores = {mainPassRenderer.renderingCompleteSemaphore()};
		vk::SubmitInfo si{
			*presentImageAvailableSemaphore,
			waitStages,
			cb,
			signalAfterRenderingSemaphores
		};
		m_device->queue().submit(si, mainPassRenderer.renderingCompleteFence());

		auto swapchain = window->swapchain();
		vk::PresentInfoKHR pi{
			signalAfterRenderingSemaphores,
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

ciallo::Project ciallo::Application::createDefaultProject() const
{
	Project project;
	entt::registry& registry = project.registry();
	// Canvas panel and drawing
	entt::entity canvasPanel = registry.create();
	entt::entity drawing = registry.create();
	auto& canvasPanelCpo = registry.emplace<CanvasPanelCpo>(canvasPanel);
	canvasPanelCpo.drawing = drawing;
	auto& vulkanImageCpo = registry.emplace<VulkanImageCpo>(drawing);
	vk::SamplerCreateInfo samplerCreateInfo{};
	vk::UniqueSampler sampler = m_device->device().createSamplerUnique(samplerCreateInfo);
	vulkanImageCpo.sampler = *sampler;

	vulkanImageCpo.image = vulkan::Image(*m_device, vulkan::MemoryAuto, vk::Format::eR8G8B8A8Unorm, 400u, 400u,
	                                     vk::SampleCountFlagBits::e1,
	                                     vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst |
	                                     vk::ImageUsageFlagBits::eColorAttachment |
	                                     vk::ImageUsageFlagBits::eTransferSrc);
	m_device->executeImmediately([&vulkanImageCpo](vk::CommandBuffer cb)
	{
		vk::ImageMemoryBarrier2 barrier = vulkanImageCpo.image.createLayoutTransitionMemoryBarrier(
			vk::ImageLayout::eGeneral);
		vk::DependencyInfo info{};
		info.setImageMemoryBarriers(barrier);
		cb.pipelineBarrier2(info);
		vulkanImageCpo.image.setImageLayout(vk::ImageLayout::eGeneral);
	});
	vk::ImageView imageView = vulkanImageCpo.image.imageView();
	vulkanImageCpo.id = ImGui_ImplVulkan_AddTexture(*sampler, imageView, VK_IMAGE_LAYOUT_GENERAL);
	auto& drawingCpo = registry.emplace<DrawingCpo>(drawing, A4Paper);
	// Drawing and layer
	entt::entity layer = registry.create();
	drawingCpo.layers.push_back(layer);



	return project;
}
