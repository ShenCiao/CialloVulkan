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

		auto data = compileShader2SPIRV(GLSLANG_STAGE_VERTEX, buffer.data(),
		                                m_filePath->filename().string().c_str());
		m_shaderModule = m_device.createShaderModuleUnique({{}, data});
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

		const glslang_input_t input = {
			.language = GLSLANG_SOURCE_GLSL,
			.stage = stage,
			.client = GLSLANG_CLIENT_VULKAN,
			.client_version = GLSLANG_TARGET_VULKAN_1_3,
			.target_language = GLSLANG_TARGET_SPV,
			.target_language_version = GLSLANG_TARGET_SPV_1_6,
			.code = shaderSource,
			.default_version = 100,
			.default_profile = GLSLANG_NO_PROFILE,
			.force_default_version_and_profile = false,
			.forward_compatible = false,
			.messages = GLSLANG_MSG_DEFAULT_BIT,
			.resource = reinterpret_cast<const glslang_resource_t*>(&DefaultTBuiltInResource)
		};
		glslang_initialize_process();

		glslang_shader_t* shader = glslang_shader_create(&input);

		if (!glslang_shader_preprocess(shader, &input))
		{
			std::cerr << result.GetErrorMessage();
			return "";
		}

		if (!glslang_shader_parse(shader, &input))
		{
			printf("GLSL parsing failed %s\n", fileName);
			printf("%s\n", glslang_shader_get_info_log(shader));
			printf("%s\n", glslang_shader_get_info_debug_log(shader));
			printf("%s\n", glslang_shader_get_preprocessed_code(shader));
			glslang_shader_delete(shader);
			return std::vector<uint32_t>();
		}

		glslang_program_t* program = glslang_program_create();
		glslang_program_add_shader(program, shader);

		if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
		{
			printf("GLSL linking failed %s\n", fileName);
			printf("%s\n", glslang_program_get_info_log(program));
			printf("%s\n", glslang_program_get_info_debug_log(program));
			glslang_program_delete(program);
			glslang_shader_delete(shader);
			return std::vector<uint32_t>();
		}

		glslang_program_SPIRV_generate(program, stage);

		std::vector<uint32_t> outShaderModule(glslang_program_SPIRV_get_size(program));
		glslang_program_SPIRV_get(program, outShaderModule.data());

		const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
		if (spirv_messages)
			printf("(%s) %s\b", fileName, spirv_messages);

		glslang_program_delete(program);
		glslang_shader_delete(shader);
		glslang_finalize_process();

		return outShaderModule;
	}
}
