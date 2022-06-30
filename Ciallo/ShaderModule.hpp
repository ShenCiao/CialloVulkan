#pragma once
#include <filesystem>
#include <shaderc/shaderc.h>

namespace ciallo::vulkan
{
	class ShaderModule
	{
		vk::Device m_device;
		vk::UniqueShaderModule m_shaderModule;
		vk::ShaderStageFlagBits m_stage;
		std::optional<std::filesystem::path> m_filePath;
	public:
		ShaderModule(vk::Device device, vk::ShaderStageFlagBits stage, const std::filesystem::path& path);
		operator vk::ShaderModule() const { return *m_shaderModule; }


	public:
		void reload();

		static std::string loadFile(const std::filesystem::path& path);
		static std::string preprocessShader(const std::string& source_name,
                              vk::ShaderStageFlagBits stage,
                              const std::string& source);
	};
}
