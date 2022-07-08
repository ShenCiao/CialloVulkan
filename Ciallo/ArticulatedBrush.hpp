#pragma once

#include <vulkan/vulkan.hpp>
#include "ShaderModule.hpp"
#include "Buffer.hpp"

namespace ciallo::rendering
{
	/**
	 * \brief For simplicity, I use a single class here for brush. Of course it's a bad design, the brush system is supposed to act like a material system.
	 */
	class Articulated
	{
		friend class CanvasRenderer;
		vk::Device m_device;
		std::unique_ptr<vulkan::ShaderModule> m_vert;
		std::unique_ptr<vulkan::ShaderModule> m_geom;
		std::unique_ptr<vulkan::ShaderModule> m_frag;
		vk::UniquePipeline m_pipeline;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
		vk::DescriptorSet m_descriptorSet;
	public:
		explicit Articulated(vk::Device device);
		void reloadShaders();
		void genDescriptorSet(vk::DescriptorPool pool, vk::Buffer uniformVert, vk::Buffer uniformFrag);
		vk::UniquePipeline createPipeline(vk::ShaderModule vert, vk::ShaderModule geom, vk::ShaderModule frag,
		                                  vk::PipelineLayout layout, vk::PipelineRenderingCreateInfo dynamicInfo);
		vk::UniquePipelineLayout createPipelineLayout(vk::DescriptorSetLayout layout);
		vk::UniqueDescriptorSetLayout createDescriptorSetLayout();
		vk::DescriptorSet createDescriptorSet(vk::DescriptorPool pool,
		                                      vk::Buffer uniformVert, vk::Buffer uniformFrag);
	};
}
