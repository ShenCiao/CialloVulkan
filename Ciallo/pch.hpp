#pragma once
#include <iostream>
#include <memory>

#include <optional>
#include <vector>
#include <unordered_set>
#include <string>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view.hpp>
#include <range/v3/range/conversion.hpp>
namespace views = ranges::views;

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>
#define ENTT_USE_ATOMIC
#include <entt/entt.hpp>

#include "GeometryPrimitives.hpp"