#ifndef PATHFINDING_GRID_HPP
#define PATHFINDING_GRID_HPP

#include "pathfinding/status.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

// the grid every algorithm searches: one graph and one cost definition, so their results
// compare. docs/architecture.md, under "the grid", states the model this file implements.
namespace pathfinding {

// the largest width or height a grid takes, so no input can freeze the tab.
inline constexpr std::int32_t max_side = 256;

// terrain weights. a wall is no weight: it is stored apart, so the grid never holds a weight
// below min_weight, and a negative or zero weight is rejected before any search sees it.
inline constexpr std::int32_t min_weight = 1;
inline constexpr std::int32_t max_weight = 9;

// a step's cost is the weight of the cell it enters times one of these. costs are integers,
// in thousandths of a straight step, so every build sums and compares them exactly alike.
// a diagonal step is sqrt(2) rounded down to 1.414 straight steps.
inline constexpr std::uint32_t straight_step_cost = 1000;
inline constexpr std::uint32_t diagonal_step_cost = 1414;

// x runs right and y down, from (0, 0) at the top left.
struct Coord {
    std::int32_t x = 0;
    std::int32_t y = 0;

    friend bool operator==(Coord, Coord) = default;
};

struct Extent {
    std::int32_t width = 0;
    std::int32_t height = 0;

    friend bool operator==(Extent, Extent) = default;
};

[[nodiscard]] bool is_valid(Extent extent) noexcept;

// 4-way moves are the orthogonal steps. 8-way adds the diagonals, under the no-corner-cutting
// rule: a diagonal step is allowed only when both orthogonal cells beside it are open, so a
// path never slips past a wall's corner or between two walls that touch at a corner.
enum class Movement : std::uint8_t {
    four_way = 0,
    eight_way = 1,
};

struct Step {
    Coord to;
    std::uint32_t cost = 0;

    friend bool operator==(Step, Step) = default;
};

// a cell's open neighbors, in a fixed order: north, east, south, west, then northeast,
// southeast, southwest, northwest. searches expand them in this order, so ties break alike.
class Neighbors {
  public:
    [[nodiscard]] std::span<const Step> steps() const noexcept {
        return std::span<const Step>(steps_).first(count_);
    }
    [[nodiscard]] std::size_t size() const noexcept { return count_; }
    [[nodiscard]] bool empty() const noexcept { return count_ == 0; }
    [[nodiscard]] auto begin() const noexcept { return steps().begin(); }
    [[nodiscard]] auto end() const noexcept { return steps().end(); }
    [[nodiscard]] const Step& operator[](std::size_t i) const noexcept { return steps()[i]; }

    // at most eight, one per direction, which Grid::neighbors keeps to.
    void push(Step step) noexcept {
        const std::span<Step> slots(steps_);
        slots[count_] = step;
        ++count_;
    }

  private:
    std::array<Step, 8> steps_{};
    std::size_t count_ = 0;
};

// a rectangular grid of open cells, each with a terrain weight, and walls, with a start and a
// goal that are always open cells inside it. a cell is stored as one byte, row by row: 0 for a
// wall and its weight otherwise, which cells() hands out as is.
class Grid {
  public:
    // a 1 x 1 open grid, its start the goal.
    Grid();

    // makes this an open grid of `extent`, every weight min_weight, with the start at the top
    // left and the goal at the bottom right. an invalid extent leaves the grid unchanged.
    [[nodiscard]] Status reset(Extent extent);

    [[nodiscard]] Extent extent() const noexcept { return extent_; }
    [[nodiscard]] std::int32_t width() const noexcept { return extent_.width; }
    [[nodiscard]] std::int32_t height() const noexcept { return extent_.height; }
    [[nodiscard]] std::size_t cell_count() const noexcept { return cells_.size(); }

    [[nodiscard]] bool contains(Coord at) const noexcept;
    // the cell's position in cells(), for a coordinate the grid contains.
    [[nodiscard]] std::size_t index(Coord at) const noexcept;
    [[nodiscard]] Coord coord(std::size_t index) const noexcept;

    [[nodiscard]] Coord start() const noexcept { return start_; }
    [[nodiscard]] Coord goal() const noexcept { return goal_; }

    // true for a wall and for anything outside the grid.
    [[nodiscard]] bool is_wall(Coord at) const noexcept;
    // the terrain weight of an open cell, and 0 for a wall or outside the grid.
    [[nodiscard]] std::int32_t weight(Coord at) const noexcept;

    // each leaves the grid unchanged unless it returns ok.
    [[nodiscard]] Status set_wall(Coord at) noexcept;                        // not on start or goal
    [[nodiscard]] Status set_weight(Coord at, std::int32_t weight) noexcept; // opens a wall
    [[nodiscard]] Status set_start(Coord at) noexcept;                       // an open cell
    [[nodiscard]] Status set_goal(Coord at) noexcept;                        // an open cell

    // the steps out of an open cell under `movement`, each costing the entered cell's weight
    // times straight_step_cost or diagonal_step_cost. none for a wall or outside the grid.
    [[nodiscard]] Neighbors neighbors(Coord from, Movement movement) const noexcept;

    [[nodiscard]] std::span<const std::uint8_t> cells() const noexcept { return cells_; }

    friend bool operator==(const Grid&, const Grid&) = default;

  private:
    Extent extent_;
    std::vector<std::uint8_t> cells_;
    Coord start_;
    Coord goal_;
};

// a 64-bit FNV-1a hash of the extent, the start, the goal and every cell, the same on every
// platform and build. equal grids hash alike, and the tests pin it for reproducible maps.
[[nodiscard]] std::uint64_t fingerprint(const Grid& grid) noexcept;

} // namespace pathfinding

#endif // PATHFINDING_GRID_HPP
