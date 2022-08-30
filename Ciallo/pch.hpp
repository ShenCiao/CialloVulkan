#pragma once
#include <iostream>
#include <memory>

#include <optional>
#include <vector>
#include <unordered_set>
#include <string>
#include <format>

#include <range/v3/view.hpp>
#include <range/v3/range/conversion.hpp>
namespace views = ranges::views;

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#define IMGUI_DEFINE_MATH_OPERATORS 
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_vulkan.h>
// -----------------------------------------------------------------------------
#define ENTT_USE_ATOMIC
#include <entt/entt.hpp>
using namespace entt::literals;
template <>
struct std::formatter<entt::entity> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(entt::entity id, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", static_cast<uint32_t>(id));
    }
};

// -----------------------------------------------------------------------------

#include "GeometryPrimitives.hpp"
