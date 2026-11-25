#include "pathfinding/probe.hpp"

namespace pathfinding::probe {

bool is_valid(Extent extent) noexcept {
    return extent.width >= 1 && extent.width <= max_side && extent.height >= 1 &&
           extent.height <= max_side;
}

std::size_t cell_count(Extent extent) noexcept {
    if (!is_valid(extent)) {
        return 0;
    }
    return static_cast<std::size_t>(extent.width) * static_cast<std::size_t>(extent.height);
}

Status fill_bands(Extent extent, std::uint32_t phase, std::span<std::uint8_t> cells) noexcept {
    if (!is_valid(extent)) {
        return Status::invalid_size;
    }
    if (cells.size() < cell_count(extent)) {
        return Status::buffer_too_small;
    }
    const auto width = static_cast<std::uint32_t>(extent.width);
    const auto height = static_cast<std::uint32_t>(extent.height);
    // the phase only matters modulo the period, so reducing it first keeps the sum small.
    const std::uint32_t offset = phase % band_period;
    std::size_t index = 0;
    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            const bool lit = (x + y + offset) % band_period < band_width;
            cells[index] = lit ? std::uint8_t{1} : std::uint8_t{0};
            ++index;
        }
    }
    return Status::ok;
}

} // namespace pathfinding::probe
