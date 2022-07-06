#pragma once
#include <filesystem>

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
		ShaderModule() = delete;

	public:
		void reload();

		static std::vector<char> loadSpv(const std::filesystem::path& path);
	};
}
