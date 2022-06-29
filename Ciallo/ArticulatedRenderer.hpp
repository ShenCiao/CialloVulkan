#pragma once

#include <vulkan/vulkan.hpp>
#include "vku.hpp"

namespace ciallo::brush::articulated
{
	class Creator
	{
		uint32_t m_width;
		uint32_t m_height;
		vk::Device m_device;
	public:
		Creator() = delete;

		vk::UniquePipeline createPipeline(uint32_t width, uint32_t height, vk::ShaderModule vert, vk::ShaderModule geom,
		                                  vk::ShaderModule frag, vk::PipelineLayout layout,
		                                  vk::PipelineRenderingCreateInfo dynamicInfo)
		{
			vku::PipelineMaker maker(width, height);
			maker.topology(vk::PrimitiveTopology::eLineStrip)
			     .shader(vk::ShaderStageFlagBits::eVertex, vert)
			     .shader(vk::ShaderStageFlagBits::eGeometry, geom)
			     .shader(vk::ShaderStageFlagBits::eFragment, frag)
			     .vertexBinding(0, 12)
			     .vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
			     .vertexAttribute(1, 0, vk::Format::eR32Sfloat, vk::blockSize(vk::Format::eR32G32Sfloat));
			return maker.createUnique(m_device, VK_NULL_HANDLE, layout, dynamicInfo);
		}

		vk::UniquePipelineLayout createPipelineLayout(vk::DescriptorSetLayout layout)
		{
			vk::PipelineLayoutCreateInfo info{};
			info.setSetLayouts(layout);
			return m_device.createPipelineLayoutUnique(info);
		}

		vk::UniqueDescriptorSetLayout createDescriptorSetLayout()
		{
			vku::DescriptorSetLayoutMaker maker{};
			maker.buffer(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1) // width
			     .buffer(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
			     // width offset curve
			     .buffer(2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1) // color
			     .buffer(3, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1);
			//opacity falloff


			return maker.createUnique(m_device);
		}

		vk::DescriptorSet createDescriptorSet(vk::DescriptorPool pool, vk::DescriptorSetLayout layout)
		{
			vk::DescriptorSetAllocateInfo info{pool, layout};
			auto descriptorSets = m_device.allocateDescriptorSets(info);

			vku::DescriptorSetUpdater updater;
		}
	};
}
