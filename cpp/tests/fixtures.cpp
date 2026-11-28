#include "fixtures.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>

namespace pathfinding::fixtures {

std::optional<std::string> read_map_text(std::string_view name) {
    std::filesystem::path path(PATHFINDING_FIXTURES_DIR);
    path /= std::string(name) + ".txt";
    // binary, so the text reaches the parser as the file holds it, line endings included.
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::nullopt;
    }
    std::string text{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    if (file.bad()) {
        return std::nullopt;
    }
    return text;
}

} // namespace pathfinding::fixtures
