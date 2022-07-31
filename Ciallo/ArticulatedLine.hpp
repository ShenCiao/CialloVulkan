#pragma once

#include "Device.hpp"
#include "Image.hpp"
#include "ShaderModule.hpp"
#include "Renderer.hpp"

namespace ciallo::rendering
{
	class ArticulatedLineEngine
	{
		vk::Device m_device;
		vulkan::ShaderModule m_vertShader;
		vulkan::ShaderModule m_fragShader;
		vulkan::ShaderModule m_geomShader;
		vk::UniquePipeline m_pipeline;
		vk::UniquePipelineLayout m_pipelineLayout;
		vulkan::Buffer m_vertBuffer;
		
	public:
		explicit ArticulatedLineEngine(vulkan::Device* device);

		void genPipelineLayout();

		void genPipelineDynamic();

		void renderDynamic(vk::CommandBuffer cb, const vulkan::Image* target);
		void genVertexBuffer(VmaAllocator allocator);
	};

	class ArticulatedLineRenderer : public Renderer
	{
		vk::Pipeline m_pipeline;
		vk::PipelineLayout m_pipelineLayout;
	public:
		ArticulatedLineRenderer() = default;
		ArticulatedLineRenderer(const ArticulatedLineRenderer& other) = default;
		ArticulatedLineRenderer(ArticulatedLineRenderer&& other) = default;
		ArticulatedLineRenderer& operator=(const ArticulatedLineRenderer& other) = default;
		ArticulatedLineRenderer& operator=(ArticulatedLineRenderer&& other) = default;
		~ArticulatedLineRenderer() override = default;

		void render(vk::CommandBuffer cb, entt::handle object) override;
	};
}