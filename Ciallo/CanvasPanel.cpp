#include "pch.hpp"
#include "CanvasPanel.hpp"

#include "Image.hpp"

namespace ciallo
{
	void CanvasPanelDrawer::update(entt::registry& r)
	{
		auto v = r.view<CanvasPanelCpo>();
		v.each([&r](entt::entity canvasPanelEntity, CanvasPanelCpo& canvasPanelCpo)
		{
			entt::entity drawingEntity = canvasPanelCpo.drawing;
			auto& vulkanImageCpo = r.get<const GPUImageCpo>(drawingEntity);
			vk::Extent2D s = vulkanImageCpo.image.extent2D();
			glm::vec2 imageSize = {
				static_cast<float>(s.width) * canvasPanelCpo.zoom,
				static_cast<float>(s.height) * canvasPanelCpo.zoom
			};

			const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_HorizontalScrollbar |
				ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar;
			bool notCloseWindow = true;
			// Padding a whole image size around middle, so it's 3 times.
			ImGui::SetNextWindowContentSize(imageSize*3.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {.0f, .0f});
			ImGui::Begin(std::format("Canvas###{}", canvasPanelEntity).c_str(), &notCloseWindow, flags);
			if (!notCloseWindow)
			{
				ImGui::End();
				ImGui::PopStyleVar(1);
				r.destroy(canvasPanelEntity);
				return;
			}

			if (!r.valid(drawingEntity) || !r.all_of<GPUImageCpo>(drawingEntity))
			{
				ImGui::Text("Image is not valid.");
				ImGui::End();
				ImGui::PopStyleVar(1);
				return;
			}
			
			auto panel = ImGui::GetCurrentWindow();
			glm::vec2 innerRectSize = panel->InnerRect.GetSize();
			glm::vec2 innerRectMin = panel->InnerRect.Min;
			glm::vec2 innerRectMinWindow = panel->InnerRect.Min - ImGui::GetWindowPos();

			ImGuiIO io = ImGui::GetIO();

			// -----------------------------------------------------------------------------
			// Mouse interaction
			ImGui::SetCursorScreenPos(innerRectMin);
			
			ImGui::InvisibleButton(std::format("CanvasInteraction##{}", canvasPanelEntity).c_str(), innerRectSize,
			                       ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight |
			                       ImGuiButtonFlags_MouseButtonMiddle);
			if (ImGui::IsItemHovered() || ImGui::IsItemActive())
			{
				if (glm::abs(io.MouseWheel) >= 1.0f)
				{
					const float wheelZoomFactor = 0.1f;
					float zoom_prev = canvasPanelCpo.zoom;
					canvasPanelCpo.zoom += wheelZoomFactor * io.MouseWheel;
					glm::vec2 mouseInnerRect = io.MousePos-innerRectMin;
					canvasPanelCpo.scroll = (canvasPanelCpo.scroll+mouseInnerRect)/zoom_prev * canvasPanelCpo.zoom - mouseInnerRect;
				}
			}

			if (ImGui::IsItemHovered())
			{
				
			}

			if (io.MouseDown[2] && ImGui::IsItemActive())
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				canvasPanelCpo.scroll = canvasPanelCpo.scroll - glm::vec2(io.MouseDelta);
			}

			canvasPanelCpo.zoom = glm::clamp(canvasPanelCpo.zoom, 0.0f, 1000.0f);
			canvasPanelCpo.scroll = glm::clamp(canvasPanelCpo.scroll, {0.0f, 0.0f}, {ImGui::GetScrollMaxX(), ImGui::GetScrollMaxY()});
			ImGui::SetScrollX(canvasPanelCpo.scroll.x);
			ImGui::SetScrollY(canvasPanelCpo.scroll.y);
			// -----------------------------------------------------------------------------
			glm::vec2 imageStartPosition = {
				imageSize.x * 3.0f >= innerRectSize.x ? imageSize.x : innerRectSize.x / 2.0f - imageSize.x / 2.0f,
				imageSize.y * 3.0f >= innerRectSize.y
					? imageSize.y + innerRectMinWindow.y
					: innerRectSize.y / 2.0f - imageSize.y / 2.0f + innerRectMinWindow.y
			};
			ImGui::SetCursorPosX(imageStartPosition.x);
			ImGui::SetCursorPosY(imageStartPosition.y);
			ImGui::Image(vulkanImageCpo.id, imageSize);

			ImGui::End();
			ImGui::PopStyleVar();
		});
	}
}
