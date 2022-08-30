#pragma once
#include "Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"

namespace ciallo
{

class Application
{
public:
	Application() = default;

	Application(const Application& other) = delete;
	Application(Application&& other) = delete;
	Application& operator=(const Application& other) = delete;
	Application& operator=(Application&& other) = delete;
	~Application() = default;

	void run();
	
	void loadSettings();
private:
	std::shared_ptr<vulkan::Instance> m_instance;
	std::shared_ptr<vulkan::Device> m_device;
};
	
}
