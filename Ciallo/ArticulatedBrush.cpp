#include "pch.hpp"
#include "ArticulatedBrush.hpp"

#include "vku.hpp"
#include "Buffer.hpp"

namespace ciallo::rendering
{
	Articulated::Articulated(vk::Device device): m_device(device)
	{
		m_vert = std::make_unique<vulkan::ShaderModule>(device, vk::ShaderStageFlagBits::eVertex,
		                                                "./shaders/articulated.vert.spv");
		m_geom = std::make_unique<vulkan::ShaderModule>(device, vk::ShaderStageFlagBits::eGeometry,
		                                                "./shaders/articulated.geom.spv");
		m_frag = std::make_unique<vulkan::ShaderModule>(device, vk::ShaderStageFlagBits::eFragment,
		                                                "./shaders/articulated.frag.spv");
		m_descriptorSetLayout = createDescriptorSetLayout();
		m_pipelineLayout = createPipelineLayout(*m_descriptorSetLayout);

		auto colorFormat = {vk::Format::eR8G8B8A8Unorm};
		vk::PipelineRenderingCreateInfo info{{}, colorFormat};
		m_pipeline = createPipeline(*m_vert, *m_geom, *m_frag, *m_pipelineLayout, info);
	}

	void Articulated::reloadShaders()
	{
		m_vert = std::make_unique<vulkan::ShaderModule>(m_device, vk::ShaderStageFlagBits::eVertex,
		                                                "./shaders/articulated.vert.spv");
		m_geom = std::make_unique<vulkan::ShaderModule>(m_device, vk::ShaderStageFlagBits::eGeometry,
		                                                "./shaders/articulated.geom.spv");
		m_frag = std::make_unique<vulkan::ShaderModule>(m_device, vk::ShaderStageFlagBits::eFragment,
		                                                "./shaders/articulated.frag.spv");
	}

	void Articulated::genDescriptorSet(vk::DescriptorPool pool, vk::Buffer uniformVert, vk::Buffer uniformFrag)
	{
		m_descriptorSet = createDescriptorSet(pool, uniformVert, uniformFrag);
	}

	vk::UniquePipeline Articulated::createPipeline(vk::ShaderModule vert, vk::ShaderModule geom, vk::ShaderModule frag,
	                                               vk::PipelineLayout layout,
	                                               vk::PipelineRenderingCreateInfo dynamicInfo)
	{
		vku::PipelineMaker maker(0, 0);
		maker.dynamicState(vk::DynamicState::eScissor)
		     .dynamicState(vk::DynamicState::eViewport)
		     .topology(vk::PrimitiveTopology::eLineStrip)
		     .shader(vk::ShaderStageFlagBits::eVertex, vert)
		     .shader(vk::ShaderStageFlagBits::eGeometry, geom)
		     .shader(vk::ShaderStageFlagBits::eFragment, frag)
		     .vertexBinding(0, blockSize(vk::Format::eR32G32Sfloat) + blockSize(vk::Format::eR32Sfloat))
		     .vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
		     .vertexAttribute(1, 0, vk::Format::eR32Sfloat, vk::blockSize(vk::Format::eR32G32Sfloat));
		return maker.createUnique(m_device, VK_NULL_HANDLE, layout, dynamicInfo);
	}

	vk::UniquePipelineLayout Articulated::createPipelineLayout(vk::DescriptorSetLayout layout)
	{
		vk::PipelineLayoutCreateInfo info{};
		info.setSetLayouts(layout);
		return m_device.createPipelineLayoutUnique(info);
	}

	vk::UniqueDescriptorSetLayout Articulated::createDescriptorSetLayout()
	{
		// width + offset curve
		// color + opacity falloff
		vku::DescriptorSetLayoutMaker maker{};
		maker.buffer(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
		     .buffer(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1);

		return maker.createUnique(m_device);
	}

	vk::DescriptorSet Articulated::createDescriptorSet(vk::DescriptorPool pool,
	                                                   vk::Buffer uniformVert, vk::Buffer uniformFrag)
	{
		vk::DescriptorSetAllocateInfo info{pool, *m_descriptorSetLayout};
		auto descriptorSets = m_device.allocateDescriptorSets(info);

		vku::DescriptorSetUpdater updater;
		updater.beginDescriptorSet(descriptorSets[0])
		       .beginBuffers(0, 0, vk::DescriptorType::eUniformBuffer)
		       .buffer(uniformVert, 0, VK_WHOLE_SIZE)
		       .beginBuffers(1, 0, vk::DescriptorType::eUniformBuffer)
		       .buffer(uniformFrag, 0, VK_WHOLE_SIZE);

		return descriptorSets[0];
	}
}
