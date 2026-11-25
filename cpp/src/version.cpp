#include "pathfinding/version.hpp"

namespace pathfinding {

std::string_view engine_version() noexcept {
    return PATHFINDING_VERSION;
}

} // namespace pathfinding
