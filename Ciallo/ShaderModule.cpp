#include "pch.hpp"
#include "ShaderModule.hpp"

#include <fstream>
#include <shaderc/shaderc.hpp>

namespace ciallo::vulkan
{
	ShaderModule::ShaderModule(vk::Device device, vk::ShaderStageFlagBits stage,
	                           const std::filesystem::path& path): m_device(device), m_stage(stage),
	                                                               m_filePath(path)
	{
		std::string buffer = loadFile(path);

		preprocessShader("test", stage, buffer);

		// shaderc::Compiler compiler;
		// auto result = compiler.CompileGlslToSpv(buffer, static_cast<shaderc_shader_kind>(stage),
		//                                         path.filename().string().c_str());
		// if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		// {
		// 	spdlog::error("Compile shader error: {}", result.GetErrorMessage());
		// 	throw std::runtime_error("fail to compile shader");
		// }

		// size_t size = std::distance(result.begin(), result.end()) * sizeof(uint32_t);
		// vk::ShaderModuleCreateInfo info{{}, size, result.begin()};
		// m_shaderModule = device.createShaderModuleUnique(info);
	}

	void ShaderModule::reload()
	{
		std::string buffer = loadFile(*m_filePath);

		shaderc::Compiler compiler;
		auto result = compiler.CompileGlslToSpv(buffer, static_cast<shaderc_shader_kind>(m_stage),
		                                        m_filePath->filename().string().c_str());
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			spdlog::error("Compile shader error: {}", result.GetErrorMessage());
			throw std::runtime_error("fail to compile shader");
		}

		size_t size = std::distance(result.begin(), result.end()) * sizeof(uint32_t);
		vk::ShaderModuleCreateInfo info{{}, size, result.begin()};

		m_shaderModule = m_device.createShaderModuleUnique(info);
	}

	std::string ShaderModule::loadFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::ate);
		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = file.tellg();
		std::string buffer;
		buffer.reserve(fileSize);
		file.seekg(0);
		file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
		return buffer;
	}

	std::string ShaderModule::preprocessShader(const std::string& source_name, vk::ShaderStageFlagBits stage,
	                                           const std::string& source)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Like -DMY_DEFINE=1
		options.AddMacroDefinition("MY_DEFINE", "1");

		shaderc::PreprocessedSourceCompilationResult result =
			compiler.PreprocessGlsl(source, shaderc_vertex_shader, source_name.c_str(), options);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::cerr << result.GetErrorMessage();
			return "";
		}

		return {result.cbegin(), result.cend()};
	}
}
