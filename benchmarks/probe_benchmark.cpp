// times the engine alone, apart from any drawing: one fill of the largest probe frame per
// iteration. usage: pathfinding_benchmark [--iterations N]
#include "pathfinding/probe.hpp"
#include "pathfinding/version.hpp"

#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

constexpr std::uint32_t default_iterations = 2000;
constexpr std::uint32_t max_iterations = 10'000'000;

bool parse_iterations(std::string_view text, std::uint32_t& out) {
    std::uint32_t value = 0;
    const char* const first = text.data();
    const char* const last = first + text.size();
    const auto [ptr, error] = std::from_chars(first, last, value);
    if (error != std::errc{} || ptr != last || value == 0 || value > max_iterations) {
        return false;
    }
    out = value;
    return true;
}

int run(std::span<char*> args) {
    namespace probe = pathfinding::probe;

    std::uint32_t iterations = default_iterations;
    if (args.size() == 3 && std::string_view(args[1]) == "--iterations") {
        if (!parse_iterations(args[2], iterations)) {
            std::cerr << "--iterations takes a whole number from 1 to " << max_iterations << '\n';
            return 2;
        }
    } else if (args.size() != 1) {
        std::cerr << "usage: pathfinding_benchmark [--iterations N]\n";
        return 2;
    }

    const probe::Extent extent{.width = probe::max_side, .height = probe::max_side};
    std::vector<std::uint8_t> cells(probe::cell_count(extent));
    std::uint64_t checksum = 0;

    const auto start = std::chrono::steady_clock::now();
    for (std::uint32_t i = 0; i < iterations; ++i) {
        if (probe::fill_bands(extent, i, cells) != probe::Status::ok) {
            std::cerr << "fill_bands failed\n";
            return 1;
        }
        checksum += cells.front(); // keeps the fill from being optimized away
    }
    const auto elapsed = std::chrono::steady_clock::now() - start;

    const auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    std::cout << "engine " << pathfinding::engine_version() << ", probe::fill_bands "
              << extent.width << "x" << extent.height << ": " << iterations << " iterations, "
              << (total_ns / iterations) << " ns each (checksum " << checksum << ")\n";
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        return run(std::span<char*>(argv, static_cast<std::size_t>(argc)));
    } catch (...) {
        // the streams can throw. there is nothing useful left to report then.
        return 1;
    }
}
