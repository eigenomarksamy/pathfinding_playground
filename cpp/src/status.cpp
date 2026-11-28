#include "pathfinding/status.hpp"

#include <string_view>

namespace pathfinding {

std::string_view to_string(Status status) noexcept {
    switch (status) {
    case Status::ok:
        return "ok";
    case Status::invalid_size:
        return "invalid_size";
    case Status::out_of_bounds:
        return "out_of_bounds";
    case Status::invalid_weight:
        return "invalid_weight";
    case Status::on_wall:
        return "on_wall";
    case Status::on_start_or_goal:
        return "on_start_or_goal";
    case Status::invalid_option:
        return "invalid_option";
    case Status::unknown_symbol:
        return "unknown_symbol";
    case Status::ragged_rows:
        return "ragged_rows";
    case Status::missing_start:
        return "missing_start";
    case Status::missing_goal:
        return "missing_goal";
    case Status::duplicate_start:
        return "duplicate_start";
    case Status::duplicate_goal:
        return "duplicate_goal";
    }
    return "unknown";
}

} // namespace pathfinding
