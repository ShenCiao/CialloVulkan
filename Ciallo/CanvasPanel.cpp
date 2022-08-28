#include "pch.hpp"
#include "CanvasPanel.hpp"

#include <filesystem>
#include <stb_image.h>

#include "Device.hpp"

namespace ciallo
{
	void CanvasPanelDrawer::update(entt::registry& registry)
	{
		auto v = registry.view<CanvasPanelCpo>();
		v.each([&registry](entt::entity canvasPanelEntity, CanvasPanelCpo& canvasPanelCpo)
		{
			const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_HorizontalScrollbar;
			bool notCloseWindow = true;
			
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {.0f, .0f});
			ImGui::Begin(std::format("Canvas###{}", canvasPanelEntity).c_str(), &notCloseWindow, flags);
			if (!notCloseWindow)
			{
				ImGui::End();
				ImGui::PopStyleVar(1);
				registry.destroy(canvasPanelEntity);
				return;
			}

			entt::entity drawingEntity = canvasPanelCpo.drawing;
			if(!registry.valid(drawingEntity) || !registry.all_of<VulkanImageCpo>(drawingEntity))
			{
				ImGui::Text("Image is not valid.");
				ImGui::End();
				ImGui::PopStyleVar(1);
				return;
			}

			auto& vulkanImageCpo = registry.get<VulkanImageCpo>(drawingEntity);
			vk::Extent2D s = vulkanImageCpo.image.extent2D();
			float sizeRatio = canvasPanelCpo.sizeRatio;
			glm::vec2 imageSize = {static_cast<float>(s.width)*sizeRatio, static_cast<float>(s.height)*sizeRatio};
			ImVec2 innerWindowSize = imageSize * 3.0f; // Padding a whole image size around middle, so it's 3 times.
			ImGui::Dummy(innerWindowSize); // Use a dummy object to fill the desired area.

			ImGui::SetCursorPos(imageSize);
			ImGui::Image(vulkanImageCpo.id, imageSize);
			ImGui::End();
			ImGui::PopStyleVar();
		});
	}
}
