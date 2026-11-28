#include "pathfinding/grid.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace pathfinding {

namespace {

constexpr std::uint8_t wall_cell = 0;

struct Direction {
    std::int32_t dx = 0;
    std::int32_t dy = 0;
};

// north, east, south, west, then northeast, southeast, southwest, northwest: Neighbors' order.
constexpr std::array<Direction, 4> orthogonal{
    Direction{.dx = 0, .dy = -1},
    Direction{.dx = 1, .dy = 0},
    Direction{.dx = 0, .dy = 1},
    Direction{.dx = -1, .dy = 0},
};
constexpr std::array<Direction, 4> diagonal{
    Direction{.dx = 1, .dy = -1},
    Direction{.dx = 1, .dy = 1},
    Direction{.dx = -1, .dy = 1},
    Direction{.dx = -1, .dy = -1},
};

constexpr Coord offset(Coord from, std::int32_t dx, std::int32_t dy) noexcept {
    return {.x = from.x + dx, .y = from.y + dy};
}

constexpr std::uint64_t fnv_offset = 14695981039346656037ULL;
constexpr std::uint64_t fnv_prime = 1099511628211ULL;

void hash_byte(std::uint64_t& hash, std::uint8_t byte) noexcept {
    hash ^= byte;
    hash *= fnv_prime;
}

// little-endian, so the hash doesn't depend on the platform's byte order.
void hash_int(std::uint64_t& hash, std::int32_t value) noexcept {
    const auto bits = static_cast<std::uint32_t>(value);
    for (std::uint32_t shift = 0; shift < 32; shift += 8) {
        hash_byte(hash, static_cast<std::uint8_t>((bits >> shift) & 0xFFU));
    }
}

} // namespace

bool is_valid(Extent extent) noexcept {
    return extent.width >= 1 && extent.width <= max_side && extent.height >= 1 &&
           extent.height <= max_side;
}

Grid::Grid() : extent_{.width = 1, .height = 1}, cells_(1, static_cast<std::uint8_t>(min_weight)) {}

Status Grid::reset(Extent extent) {
    if (!is_valid(extent)) {
        return Status::invalid_size;
    }
    extent_ = extent;
    cells_.assign(static_cast<std::size_t>(extent.width) * static_cast<std::size_t>(extent.height),
                  static_cast<std::uint8_t>(min_weight));
    start_ = {.x = 0, .y = 0};
    goal_ = {.x = extent.width - 1, .y = extent.height - 1};
    return Status::ok;
}

bool Grid::contains(Coord at) const noexcept {
    return at.x >= 0 && at.x < extent_.width && at.y >= 0 && at.y < extent_.height;
}

std::size_t Grid::index(Coord at) const noexcept {
    return (static_cast<std::size_t>(at.y) * static_cast<std::size_t>(extent_.width)) +
           static_cast<std::size_t>(at.x);
}

Coord Grid::coord(std::size_t index) const noexcept {
    const auto width = static_cast<std::size_t>(extent_.width);
    return {
        .x = static_cast<std::int32_t>(index % width),
        .y = static_cast<std::int32_t>(index / width),
    };
}

bool Grid::is_wall(Coord at) const noexcept {
    return !contains(at) || cells_[index(at)] == wall_cell;
}

std::int32_t Grid::weight(Coord at) const noexcept {
    if (!contains(at)) {
        return 0;
    }
    return cells_[index(at)];
}

Status Grid::set_wall(Coord at) noexcept {
    if (!contains(at)) {
        return Status::out_of_bounds;
    }
    if (at == start_ || at == goal_) {
        return Status::on_start_or_goal;
    }
    cells_[index(at)] = wall_cell;
    return Status::ok;
}

Status Grid::set_weight(Coord at, std::int32_t weight) noexcept {
    if (!contains(at)) {
        return Status::out_of_bounds;
    }
    if (weight < min_weight || weight > max_weight) {
        return Status::invalid_weight;
    }
    cells_[index(at)] = static_cast<std::uint8_t>(weight);
    return Status::ok;
}

Status Grid::set_start(Coord at) noexcept {
    if (!contains(at)) {
        return Status::out_of_bounds;
    }
    if (is_wall(at)) {
        return Status::on_wall;
    }
    start_ = at;
    return Status::ok;
}

Status Grid::set_goal(Coord at) noexcept {
    if (!contains(at)) {
        return Status::out_of_bounds;
    }
    if (is_wall(at)) {
        return Status::on_wall;
    }
    goal_ = at;
    return Status::ok;
}

Neighbors Grid::neighbors(Coord from, Movement movement) const noexcept {
    Neighbors result;
    if (is_wall(from)) {
        return result;
    }
    for (const Direction d : orthogonal) {
        const Coord to = offset(from, d.dx, d.dy);
        if (!is_wall(to)) {
            const auto cost = static_cast<std::uint32_t>(weight(to)) * straight_step_cost;
            result.push({.to = to, .cost = cost});
        }
    }
    if (movement == Movement::eight_way) {
        for (const Direction d : diagonal) {
            const Coord to = offset(from, d.dx, d.dy);
            // no corner cutting: both orthogonal cells beside the diagonal must be open.
            const bool corner_open =
                !is_wall(offset(from, d.dx, 0)) && !is_wall(offset(from, 0, d.dy));
            if (corner_open && !is_wall(to)) {
                const auto cost = static_cast<std::uint32_t>(weight(to)) * diagonal_step_cost;
                result.push({.to = to, .cost = cost});
            }
        }
    }
    return result;
}

std::uint64_t fingerprint(const Grid& grid) noexcept {
    std::uint64_t hash = fnv_offset;
    hash_int(hash, grid.width());
    hash_int(hash, grid.height());
    hash_int(hash, grid.start().x);
    hash_int(hash, grid.start().y);
    hash_int(hash, grid.goal().x);
    hash_int(hash, grid.goal().y);
    for (const std::uint8_t cell : grid.cells()) {
        hash_byte(hash, cell);
    }
    return hash;
}

} // namespace pathfinding
