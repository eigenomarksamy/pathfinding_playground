#include "pathfinding/generate.hpp"

#include <cstdint>
#include <utility>

namespace pathfinding {

std::uint64_t SplitMix64::next() noexcept {
    state_ += 0x9E3779B97F4A7C15ULL;
    std::uint64_t z = state_;
    z = (z ^ (z >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27U)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31U);
}

std::uint32_t SplitMix64::below(std::uint32_t bound) noexcept {
    if (bound == 0) {
        return 0;
    }
    const std::uint64_t range = bound;
    // 2^64 mod range: drawing again below it leaves a whole number of copies of the range.
    const std::uint64_t threshold = (std::uint64_t{0} - range) % range;
    std::uint64_t value = next();
    while (value < threshold) {
        value = next();
    }
    return static_cast<std::uint32_t>(value % range);
}

bool is_valid(const Generation& options) noexcept {
    return is_valid(options.extent) && options.wall_percent >= 0 &&
           options.wall_percent <= max_percent && options.weighted_percent >= 0 &&
           options.weighted_percent <= max_percent - options.wall_percent;
}

Status generate(const Generation& options, Grid& out) {
    if (!is_valid(options.extent)) {
        return Status::invalid_size;
    }
    if (!is_valid(options)) {
        return Status::invalid_option;
    }
    Grid grid;
    if (const Status status = grid.reset(options.extent); status != Status::ok) {
        return status;
    }
    SplitMix64 random(options.seed);
    const auto walls = static_cast<std::uint32_t>(options.wall_percent);
    const auto weighted = walls + static_cast<std::uint32_t>(options.weighted_percent);
    const auto heavier = static_cast<std::uint32_t>(max_weight - min_weight);
    // every cell takes its draws in the same order, the start and the goal included, so one
    // cell's terrain never shifts another's.
    for (std::int32_t y = 0; y < grid.height(); ++y) {
        for (std::int32_t x = 0; x < grid.width(); ++x) {
            const Coord at{.x = x, .y = y};
            const std::uint32_t roll = random.below(static_cast<std::uint32_t>(max_percent));
            const std::uint32_t extra = random.below(heavier);
            if (at == grid.start() || at == grid.goal()) {
                continue;
            }
            Status status = Status::ok;
            if (roll < walls) {
                status = grid.set_wall(at);
            } else if (roll < weighted) {
                status = grid.set_weight(at, min_weight + 1 + static_cast<std::int32_t>(extra));
            }
            if (status != Status::ok) {
                return status;
            }
        }
    }
    out = std::move(grid);
    return Status::ok;
}

} // namespace pathfinding
