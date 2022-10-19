#pragma once
#include "Instance.hpp"
#include "Device.hpp"
#include "Project.hpp"

namespace ciallo
{

class Application
{
public:
	Application() = default;

	Application(const Application& other) = delete;
	Application(Application&& other) = default;
	Application& operator=(const Application& other) = delete;
	Application& operator=(Application&& other) = default;
	~Application() = default;

	void run();

	Project createDefaultProject() const;
private:
	std::shared_ptr<vulkan::Instance> m_instance;
	std::shared_ptr<vulkan::Device> m_device;
};
	
}
