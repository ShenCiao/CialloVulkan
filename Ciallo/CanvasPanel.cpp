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
			const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_HorizontalScrollbar |
				ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar;
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
			if (!registry.valid(drawingEntity) || !registry.all_of<VulkanImageCpo>(drawingEntity))
			{
				ImGui::Text("Image is not valid.");
				ImGui::End();
				ImGui::PopStyleVar(1);
				return;
			}
			auto& vulkanImageCpo = registry.get<const VulkanImageCpo>(drawingEntity);
			vk::Extent2D s = vulkanImageCpo.image.extent2D();
			glm::vec2 imageSize = {
				static_cast<float>(s.width) * canvasPanelCpo.zoom,
				static_cast<float>(s.height) * canvasPanelCpo.zoom
			};
			// coordinate in world space, viewRect's origin at window left up corner, not include title bar and scroll bar
			// window origin != work rect origin
			glm::vec2 viewRectSize = ImGui::GetContentRegionAvail();
			glm::vec2 viewRectOriginWindow = glm::vec2{0.f, ImGui::GetFrameHeight()}; // add menu if need
			glm::vec2 viewRectOrigin = viewRectOriginWindow + glm::vec2(ImGui::GetWindowPos());

			{
				ImVec2 vMin = viewRectOrigin;
				ImVec2 vMax = viewRectOrigin + viewRectSize;

				ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(255, 255, 0, 255));
			}

			ImGuiIO io = ImGui::GetIO();

			// -----------------------------------------------------------------------------
			// Mouse interaction
			if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(viewRectOrigin, viewRectOrigin + viewRectSize))
			{
				if (glm::abs(io.MouseWheel) >= 0.1f)
				{
					spdlog::info("MouseWheel: {}", io.MouseWheel);
					const float wheelZoomFactor = 0.05f;
					float zoom_prev = canvasPanelCpo.zoom;
					canvasPanelCpo.zoom += wheelZoomFactor * io.MouseWheel;

					glm::vec2 mousePos = io.MousePos;
					glm::vec2 mouseViewRectRatio = (mousePos - viewRectOrigin) / viewRectSize;
					glm::vec2 mouseContentPos = glm::vec2{ImGui::GetScrollX(), ImGui::GetScrollY()} + (mousePos -
						viewRectOrigin);
					ImGui::SetScrollX(ImGui::GetScrollX() / zoom_prev * canvasPanelCpo.zoom);
					ImGui::SetScrollY(ImGui::GetScrollY() / zoom_prev * canvasPanelCpo.zoom);
				}
			}
			// -----------------------------------------------------------------------------


			// Padding a whole image size around middle, so it's 3 times.
			ImGui::Dummy(imageSize * 3.0f); // Use a dummy object to fill the desired area.

			glm::vec2 imageStartPosition = {
				imageSize.x * 3.0f >= viewRectSize.x ? imageSize.x : viewRectSize.x / 2.0f - imageSize.x / 2.0f,
				imageSize.y * 3.0f >= viewRectSize.y
					? imageSize.y + viewRectOriginWindow.y
					: viewRectSize.y / 2.0f - imageSize.y / 2.0f + viewRectOriginWindow.y
			};
			ImGui::SetCursorPosX(imageStartPosition.x);
			ImGui::SetCursorPosY(imageStartPosition.y);

			ImGui::Image(vulkanImageCpo.id, imageSize);
			ImGui::End();
			ImGui::PopStyleVar();
		});
	}
}
