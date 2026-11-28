#ifndef PATHFINDING_TESTS_FIXTURES_HPP
#define PATHFINDING_TESTS_FIXTURES_HPP

#include "pathfinding/generate.hpp"
#include "pathfinding/grid.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// the canonical test maps every algorithm is tested on. each is a map text file in
// cpp/tests/fixtures/maps/, except the large one, which is generated from a fixed seed, so a
// 256 x 256 map isn't committed by hand. docs/architecture.md lists what each one checks.
namespace pathfinding::fixtures {

inline constexpr std::array<std::string_view, 7> map_names{
    "open",          // no walls, weight 1 everywhere
    "walls",         // a wall with one gap
    "weighted",      // heavy terrain across the straight route, with an open detour
    "start_is_goal", // the start and the goal on one cell
    "no_path",       // the goal walled in
    "blocked",       // a diagonal line of walls only corner cutting could cross
    "tiny",          // 2 x 1
};

// the large map: the largest grid, a quarter of it walls and some of it weighted.
inline constexpr Generation large_map{
    .extent = {max_side, max_side},
    .seed = 20261128,
    .wall_percent = 25,
    .weighted_percent = 15,
};

// its fingerprint, the same on every platform and build. a change to it is a change to the
// generator or the grid's layout, and every trace recorded on the map changes with it.
inline constexpr std::uint64_t large_map_fingerprint = 0xb8f15fb9075471aaULL;

// the text of the named map's file, or nothing when it can't be read.
[[nodiscard]] std::optional<std::string> read_map_text(std::string_view name);

} // namespace pathfinding::fixtures

#endif // PATHFINDING_TESTS_FIXTURES_HPP
