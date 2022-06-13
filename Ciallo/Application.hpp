#pragma once
#include "Window.hpp"

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

	void run() const;
	
	void loadSettings();

private:
	int m_mainWindowWidth;
	int m_mainWindowHeight;
	std::unique_ptr<vulkan::Window> m_mainWindow;
};
	
}
