#include "pch.hpp"
#include "MainPassRenderer.hpp"

namespace ciallo::vulkan
{
	MainPassRenderer::MainPassRenderer(Window* const w): w(w)
	{
		ImGui::CreateContext();
	}

	MainPassRenderer::~MainPassRenderer()
	{
		ImGui::DestroyContext();
	}

	void MainPassRenderer::imguiCheckVkResult(VkResult err)
	{
		if (err == 0)
			return;
		spdlog::error("[vulkan] Error: VkResult = {:03d}", err);
		if (err < 0)
			abort();
	}

	void MainPassRenderer::initImGui()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		w->imguiInitWindow();

		createDescriptorPool();
		ImGui_ImplVulkan_InitInfo init_info{};
		init_info.Instance = w->instance();
		init_info.PhysicalDevice = w->physicalDevice();
		init_info.Device = w->device();
		init_info.QueueFamily = w->queueFamilyIndex();
		init_info.Queue = w->queue();
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = *m_descriptorPool;
		init_info.Subpass = 0;
		init_info.MinImageCount = 3;
		init_info.ImageCount = w->swapchainImageCount();
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = imguiCheckVkResult;
		ImGui_ImplVulkan_Init(&init_info, w->renderPass());
	}

	void MainPassRenderer::createDescriptorPool()
	{
		vk::DescriptorPoolCreateInfo poolInfo(
			{},
			MAX_SIZE * static_cast<uint32_t>(m_descriptorPoolSizes.size()),
			m_descriptorPoolSizes
		);
		m_descriptorPool = w->device().createDescriptorPoolUnique(poolInfo);
	}
}
