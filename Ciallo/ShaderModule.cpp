#include "pch.hpp"
#include "ShaderModule.hpp"

#include <fstream>
#include <glslang/Public/ShaderLang.h>

namespace ciallo::vulkan
{
	ShaderModule::ShaderModule(vk::Device device, vk::ShaderStageFlagBits stage,
	                           const std::filesystem::path& path): m_device(device), m_shaderStage(stage), m_filePath(path)
	{
		std::vector<char> buffer = loadFile(path);
		auto data = compileShader2SPIRV(static_cast<glslang_stage_t>(stage), buffer.data(),
		                                path.filename().string().c_str());

		vk::ShaderModuleCreateInfo info{{}, data};
		m_shaderModule = device.createShaderModuleUnique(info);
	}

	void ShaderModule::reload()
	{
		std::vector<char> buffer = loadFile(*m_filePath);
			
		auto data = compileShader2SPIRV(static_cast<glslang_stage_t>(m_shaderStage), buffer.data(),
		                                m_filePath->filename().string().c_str());
		m_shaderModule = m_device.createShaderModuleUnique({{}, data});
	}

	std::vector<char> ShaderModule::loadFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::ate);
		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
		return buffer;
	}

	// Being copied from https://github.com/KhronosGroup/glslang#c-functional-interface-new
	// change client_version, target_language_version, delete resource inside `input`
	std::vector<uint32_t> ShaderModule::compileShader2SPIRV(glslang_stage_t stage,
	                                                        const char* shaderSource, const char* fileName)
	{
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
		};

		glslang_shader_t* shader = glslang_shader_create(&input);

		if (!glslang_shader_preprocess(shader, &input))
		{
			printf("GLSL preprocessing failed %s\n", fileName);
			printf("%s\n", glslang_shader_get_info_log(shader));
			printf("%s\n", glslang_shader_get_info_debug_log(shader));
			printf("%s\n", input.code);
			glslang_shader_delete(shader);
			return std::vector<uint32_t>();
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

		return outShaderModule;
	}
}
