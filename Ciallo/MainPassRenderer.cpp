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
		if (m_imguiInitialized)
		{
			ImGui_ImplVulkan_Shutdown();
			w->imguiShutdownWindow();
		}
		ImGui::DestroyContext();
	}

	void MainPassRenderer::init()
	{
		createRenderPass();
		createFramebuffers();
		initImGui();
		createSyncObject();
		uploadFonts();
	}

	void MainPassRenderer::render(vk::CommandBuffer cb, uint32_t framebufferIndex, ImDrawData* drawData)
	{
		vk::ClearValue v{};
		vk::ClearColorValue cv{};
		cv.setFloat32({.0f, .0f, .0f, 1.0f});
		v.setColor(cv);
		vk::RenderPassBeginInfo rpbi(*m_renderPass, *m_framebuffers[framebufferIndex], {{0, 0}, w->swapchainExtent()},
		                             v);
		cb.beginRenderPass(rpbi, vk::SubpassContents::eInline);
		ImGui_ImplVulkan_RenderDrawData(drawData, cb);
		cb.endRenderPass();
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
		// ReSharper disable once CppInitializedValueIsAlwaysRewritten
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
		m_imguiInitialized = ImGui_ImplVulkan_Init(&init_info, *m_renderPass);
		if (!m_imguiInitialized)
		{
			throw std::runtime_error("Cannot initialize imgui!");
		}
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

	void MainPassRenderer::createSyncObject()
	{
		vk::FenceCreateInfo fci(vk::FenceCreateFlagBits::eSignaled);
		m_renderingCompleteFence = w->device().createFenceUnique(fci);
		vk::SemaphoreCreateInfo sci({});
		m_renderingCompleteSemaphore = w->device().createSemaphoreUnique(sci);
	}

	void MainPassRenderer::createRenderPass()
	{
		vku::RenderpassMaker rpm;
		rpm.attachmentBegin(w->swapchainImageFormat())
		   .attachmentLoadOp(vk::AttachmentLoadOp::eClear)
		   .attachmentStoreOp(vk::AttachmentStoreOp::eStore)
		   .attachmentFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		rpm.subpassBegin(vk::PipelineBindPoint::eGraphics)
		   .subpassColorAttachment(vk::ImageLayout::eAttachmentOptimal, 0);
		m_renderPass = rpm.createUnique(w->device());
	}

	void MainPassRenderer::uploadFonts() const
	{
		w->executeImmediately([this](vk::CommandBuffer cb)
		{
			cb.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
			ImGui_ImplVulkan_CreateFontsTexture(cb);
			cb.end();
		});
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	/**
	 * \brief Create or recreate Framebuffers
	 */
	void MainPassRenderer::createFramebuffers()
	{
		auto imageView2framebuffer = [this](const vk::UniqueImageView& imv)
		{
			vk::FramebufferCreateInfo info{
				{},
				*m_renderPass,
				*imv,
				w->swapchainExtent().width,
				w->swapchainExtent().height,
				1
			};
			return w->device().createFramebufferUnique(info);
		};
		m_framebuffers = w->m_swapchainImageViews | views::transform(imageView2framebuffer) | ranges::to_vector;
	}
}
