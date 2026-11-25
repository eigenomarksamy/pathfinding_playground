#pragma once

#include <string_view>

namespace pathfinding {

// the engine's version, the project's version in CMakeLists.txt.
[[nodiscard]] std::string_view engine_version() noexcept;

} // namespace pathfinding
