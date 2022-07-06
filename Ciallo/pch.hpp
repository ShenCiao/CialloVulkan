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

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>