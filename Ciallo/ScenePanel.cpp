#include "pch.hpp"
#include "ScenePanel.hpp"

namespace ciallo::gui
{
	void ScenePanel::draw()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_HorizontalScrollbar;
		ImGui::Begin("Scene", nullptr , flags);
		
	}
}
