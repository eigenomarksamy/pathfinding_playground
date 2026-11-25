// the engine's interface to JavaScript. it stays small and batched: one call returns a whole
// frame, never one call per cell. web/src/engine/engine.ts is the typed adapter over it.
#include "pathfinding/probe.hpp"
#include "pathfinding/version.hpp"

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <cstdint>
#include <string>
#include <vector>

namespace {

namespace probe = pathfinding::probe;

struct BandsResult {
    probe::Status status = probe::Status::ok;
    emscripten::val cells = emscripten::val::null(); // a Uint8Array the caller owns, or null
};

std::string engine_version() {
    return std::string(pathfinding::engine_version());
}

BandsResult bands(std::int32_t width, std::int32_t height, std::uint32_t phase) {
    const probe::Extent extent{.width = width, .height = height};
    if (!probe::is_valid(extent)) {
        return {probe::Status::invalid_size, emscripten::val::null()};
    }
    // reused between calls, so a frame allocates only when the extent grows.
    static std::vector<std::uint8_t> buffer;
    buffer.resize(probe::cell_count(extent));
    const probe::Status status = probe::fill_bands(extent, phase, buffer);
    if (status != probe::Status::ok) {
        return {status, emscripten::val::null()};
    }
    // one copy of the frame into memory JavaScript owns, so the next call can't change it.
    const auto view = emscripten::typed_memory_view(buffer.size(), buffer.data());
    return {status, emscripten::val::global("Uint8Array").new_(view)};
}

} // namespace

EMSCRIPTEN_BINDINGS(pathfinding) {
    using namespace emscripten;
    enum_<probe::Status>("Status")
        .value("ok", probe::Status::ok)
        .value("invalid_size", probe::Status::invalid_size)
        .value("buffer_too_small", probe::Status::buffer_too_small);
    value_object<BandsResult>("BandsResult")
        .field("status", &BandsResult::status)
        .field("cells", &BandsResult::cells);
    constant("maxSide", probe::max_side);
    function("engineVersion", &engine_version);
    function("bands", &bands);
}
