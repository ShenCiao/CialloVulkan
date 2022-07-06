#include "pch.hpp"
#include "ShaderModule.hpp"

#include <fstream>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace ciallo::vulkan
{
	ShaderModule::ShaderModule(vk::Device device, vk::ShaderStageFlagBits stage,
	                           const std::filesystem::path& path): m_device(device), m_stage(stage),
	                                                               m_filePath(path)
	{
		auto buffer = loadSpv(path);
		vk::ShaderModuleCreateInfo info{{}, buffer.size(), reinterpret_cast<const uint32_t*>(buffer.data())};
		m_shaderModule = device.createShaderModuleUnique(info);
	}

	void ShaderModule::reload()
	{
		auto buffer = loadSpv(*m_filePath);
		vk::ShaderModuleCreateInfo info{{}, buffer.size(), reinterpret_cast<const uint32_t*>(buffer.data())};
		m_shaderModule = m_device.createShaderModuleUnique(info);
	}

	std::vector<char> ShaderModule::loadSpv(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::ate|std::ios::binary);
		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}

	static EShLanguage findLanguage(const vk::ShaderStageFlagBits shaderType)
	{
		switch (shaderType)
		{
		case vk::ShaderStageFlagBits::eVertex:
			return EShLangVertex;
		case vk::ShaderStageFlagBits::eTessellationControl:
			return EShLangTessControl;
		case vk::ShaderStageFlagBits::eTessellationEvaluation:
			return EShLangTessEvaluation;
		case vk::ShaderStageFlagBits::eGeometry:
			return EShLangGeometry;
		case vk::ShaderStageFlagBits::eFragment:
			return EShLangFragment;
		case vk::ShaderStageFlagBits::eCompute:
			return EShLangCompute;
		default:
			throw std::runtime_error("Shader type unsupported");
		}
	}
}
