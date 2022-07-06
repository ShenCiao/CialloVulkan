#pragma once

#include <vulkan/vulkan.hpp>
#include "ShaderModule.hpp"

namespace ciallo::brush::articulated
{
	/**
	 * \brief For simplicity, I use a renderer class here for brush. Of course it's a bad design, the brush system is supposed to act like a material system.
	 */
	class Renderer
	{
		vk::Device m_device;
		std::unique_ptr<vulkan::ShaderModule> m_vert;
		std::unique_ptr<vulkan::ShaderModule> m_geom;
		std::unique_ptr<vulkan::ShaderModule> m_frag;
		vk::UniquePipeline m_pipeline;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
		vk::UniqueDescriptorSet m_descriptorSet;
	public:
		explicit Renderer(vk::Device device);
		void reloadShaders();
		vk::UniquePipeline createPipeline(vk::ShaderModule vert, vk::ShaderModule geom, vk::ShaderModule frag,
		                                  vk::PipelineLayout layout, vk::PipelineRenderingCreateInfo dynamicInfo);
		vk::UniquePipelineLayout createPipelineLayout(vk::DescriptorSetLayout layout);
		vk::UniqueDescriptorSetLayout createDescriptorSetLayout();
		vk::DescriptorSet createDescriptorSet(vk::DescriptorPool pool,
		                                      vk::Buffer bufferVert, vk::Buffer bufferFrag);
	};
}
