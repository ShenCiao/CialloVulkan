#pragma once
#include <filesystem>
#include <glslang/Include/glslang_c_interface.h>

namespace ciallo::vulkan
{
	class ShaderModule
	{
		vk::Device m_device;
		vk::UniqueShaderModule m_shaderModule;
		vk::ShaderStageFlagBits m_shaderStage;
		std::optional<std::filesystem::path> m_filePath;
	public:
		ShaderModule(vk::Device device, vk::ShaderStageFlagBits stage, const std::filesystem::path& path);
		operator vk::ShaderModule() const { return *m_shaderModule; }


	public:
		void reload();

		static std::vector<char> loadFile(const std::filesystem::path& path);
		static std::vector<uint32_t> compileShader2SPIRV(glslang_stage_t stage, const char* shaderSource,
		                                                 const char* fileName);
	};
}
