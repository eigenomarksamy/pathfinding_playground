#ifndef PATHFINDING_STATUS_HPP
#define PATHFINDING_STATUS_HPP

#include <cstdint>
#include <string_view>

namespace pathfinding {

// what the grid, the map text and the generator return instead of throwing: the WebAssembly
// module is built without C++ exceptions. the values are stable, since M9 hands them to the app.
enum class Status : std::uint8_t {
    ok = 0,
    invalid_size = 1,     // a side is below 1 or above max_side
    out_of_bounds = 2,    // a coordinate outside the grid
    invalid_weight = 3,   // a terrain weight outside min_weight to max_weight, negatives included
    on_wall = 4,          // the start or the goal placed on a wall
    on_start_or_goal = 5, // a wall placed on the start or the goal
    invalid_option = 6,   // a generation option out of its range
    unknown_symbol = 7,   // map text: a character the format doesn't define
    ragged_rows = 8,      // map text: a row whose width differs from the first row's
    missing_start = 9,    // map text: no start
    missing_goal = 10,    // map text: no goal
    duplicate_start = 11, // map text: a second start
    duplicate_goal = 12,  // map text: a second goal
};

// the status's name, as the enum spells it.
[[nodiscard]] std::string_view to_string(Status status) noexcept;

} // namespace pathfinding

#endif // PATHFINDING_STATUS_HPP
