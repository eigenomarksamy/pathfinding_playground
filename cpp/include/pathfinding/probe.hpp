#ifndef PATHFINDING_PROBE_HPP
#define PATHFINDING_PROBE_HPP

#include <cstddef>
#include <cstdint>
#include <span>

// the bootstrap's stand-in for the engine: a pattern of diagonal bands that moves one cell
// per step. it exercises the native and WebAssembly builds, the bounds checks, and the
// batched transfer of a whole frame to the app. M1's grid replaces it.
namespace pathfinding::probe {

// the largest width or height the probe accepts, so no input can freeze the tab.
inline constexpr std::int32_t max_side = 256;

// cells repeat every `band_period` diagonals, and the first `band_width` of them are lit.
inline constexpr std::uint32_t band_period = 8;
inline constexpr std::uint32_t band_width = 3;

enum class Status : std::uint8_t {
    ok = 0,
    invalid_size = 1,     // a side is below 1 or above max_side
    buffer_too_small = 2, // the output holds fewer than width * height cells
};

struct Extent {
    std::int32_t width = 0;
    std::int32_t height = 0;
};

[[nodiscard]] bool is_valid(Extent extent) noexcept;

// width * height for a valid extent, and 0 for an invalid one.
[[nodiscard]] std::size_t cell_count(Extent extent) noexcept;

// fills the first cell_count(extent) cells row by row, 1 where a band lies and 0 elsewhere.
// a cell (x, y) is lit when (x + y + phase) mod band_period < band_width, so each step of
// the phase moves the bands one diagonal. cells past cell_count(extent) are left alone.
[[nodiscard]] Status fill_bands(Extent extent, std::uint32_t phase,
                                std::span<std::uint8_t> cells) noexcept;

} // namespace pathfinding::probe

#endif // PATHFINDING_PROBE_HPP