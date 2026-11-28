#ifndef PATHFINDING_GENERATE_HPP
#define PATHFINDING_GENERATE_HPP

#include "pathfinding/grid.hpp"
#include "pathfinding/status.hpp"

#include <cstdint>

// reproducible map generation: the same options, seed included, give the same grid on every
// platform and build. the standard library's distributions differ between implementations, so
// the generator and its bounded draws are this file's own.
namespace pathfinding {

// SplitMix64, by Sebastiano Vigna: small, fast and fully specified by its seed.
class SplitMix64 {
  public:
    explicit SplitMix64(std::uint64_t seed) noexcept : state_(seed) {}

    [[nodiscard]] std::uint64_t next() noexcept;

    // a uniform draw from 0 to bound - 1, without modulo bias. a bound of 0 gives 0.
    [[nodiscard]] std::uint32_t below(std::uint32_t bound) noexcept;

  private:
    std::uint64_t state_;
};

inline constexpr std::int32_t max_percent = 100;

// a random map. each cell, row by row, is a wall with `wall_percent` chance, otherwise weighted
// terrain of weight 2 to max_weight with `weighted_percent` chance, and otherwise of weight 1.
// the start is the top left cell and the goal the bottom right, both open and of weight 1. a
// path between them isn't guaranteed: maze generation is a later ticket's.
struct Generation {
    Extent extent;
    std::uint64_t seed = 0;
    std::int32_t wall_percent = 0;     // 0 to max_percent
    std::int32_t weighted_percent = 0; // 0 to max_percent - wall_percent
};

[[nodiscard]] bool is_valid(const Generation& options) noexcept;

// generates the map into `out`, which is left unchanged unless the status is ok: invalid_size
// for the extent, and invalid_option for the percentages.
[[nodiscard]] Status generate(const Generation& options, Grid& out);

} // namespace pathfinding

#endif // PATHFINDING_GENERATE_HPP
