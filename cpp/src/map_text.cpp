#include "pathfinding/map_text.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace pathfinding {

namespace {

constexpr char open_symbol = '.';
constexpr char wall_symbol = '#';
constexpr char start_symbol = 'S';
constexpr char goal_symbol = 'G';
constexpr char start_and_goal_symbol = '*';

constexpr Coord cell_at(std::size_t x, std::size_t y) noexcept {
    return {.x = static_cast<std::int32_t>(x), .y = static_cast<std::int32_t>(y)};
}

constexpr MapParse failure(Status status, Coord at) noexcept {
    return {.status = status, .at = at};
}

constexpr MapParse success() noexcept {
    return {.status = Status::ok, .at = {}};
}

// the next line of `text` from `offset`, without its LF or CRLF, and the offset past it.
std::string_view next_line(std::string_view text, std::size_t& offset) noexcept {
    const std::size_t end = text.find('\n', offset);
    std::string_view line =
        end == std::string_view::npos ? text.substr(offset) : text.substr(offset, end - offset);
    offset = end == std::string_view::npos ? text.size() : end + 1;
    if (!line.empty() && line.back() == '\r') {
        line.remove_suffix(1);
    }
    return line;
}

// the weight a symbol gives an open cell, 0 for a wall, and nothing for an unknown symbol.
std::optional<std::int32_t> symbol_weight(char symbol) noexcept {
    if (symbol == wall_symbol) {
        return 0;
    }
    if (symbol == open_symbol || symbol == start_symbol || symbol == goal_symbol ||
        symbol == start_and_goal_symbol) {
        return min_weight;
    }
    if (symbol >= '0' + min_weight + 1 && symbol <= '0' + max_weight) {
        return symbol - '0';
    }
    return std::nullopt;
}

// splits the text into rows, and checks the grid's height and its first row's width.
MapParse split_rows(std::string_view text, std::vector<std::string_view>& rows) {
    const auto most = static_cast<std::size_t>(max_side);
    std::size_t offset = 0;
    while (offset < text.size()) {
        if (rows.size() == most) {
            return failure(Status::invalid_size, cell_at(0, most));
        }
        rows.push_back(next_line(text, offset));
    }
    if (rows.empty() || rows.front().empty()) {
        return failure(Status::invalid_size, {});
    }
    if (rows.front().size() > most) {
        return failure(Status::invalid_size, cell_at(most, 0));
    }
    return success();
}

struct Ends {
    std::optional<Coord> start;
    std::optional<Coord> goal;
};

// marks the start or the goal, refusing a second one.
MapParse mark(std::optional<Coord>& end, Coord at, Status duplicate) noexcept {
    if (end) {
        return failure(duplicate, at);
    }
    end = at;
    return success();
}

// checks every row's width and symbols, and finds the start and the goal, refusing a second.
MapParse scan_rows(const std::vector<std::string_view>& rows, Ends& ends) {
    const std::size_t width = rows.front().size();
    for (std::size_t y = 0; y < rows.size(); ++y) {
        const std::string_view row = rows[y];
        if (row.size() != width) {
            return failure(Status::ragged_rows, cell_at(std::min(row.size(), width), y));
        }
        for (std::size_t x = 0; x < width; ++x) {
            const char symbol = row[x];
            const Coord at = cell_at(x, y);
            MapParse result = success();
            if (!symbol_weight(symbol)) {
                result = failure(Status::unknown_symbol, at);
            } else if (symbol == start_symbol) {
                result = mark(ends.start, at, Status::duplicate_start);
            } else if (symbol == goal_symbol) {
                result = mark(ends.goal, at, Status::duplicate_goal);
            } else if (symbol == start_and_goal_symbol) {
                result = mark(ends.start, at, Status::duplicate_start);
                if (result.status == Status::ok) {
                    result = mark(ends.goal, at, Status::duplicate_goal);
                }
            }
            if (result.status != Status::ok) {
                return result;
            }
        }
    }
    return success();
}

// builds the grid through its own checks, which the scanned rows already pass.
MapParse build(const std::vector<std::string_view>& rows, Coord start, Coord goal, Grid& grid) {
    const std::size_t width = rows.front().size();
    const Extent extent{
        .width = static_cast<std::int32_t>(width),
        .height = static_cast<std::int32_t>(rows.size()),
    };
    if (const Status status = grid.reset(extent); status != Status::ok) {
        return failure(status, {});
    }
    if (const Status status = grid.set_start(start); status != Status::ok) {
        return failure(status, start);
    }
    if (const Status status = grid.set_goal(goal); status != Status::ok) {
        return failure(status, goal);
    }
    for (std::size_t y = 0; y < rows.size(); ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            const Coord at = cell_at(x, y);
            const std::int32_t weight = symbol_weight(rows[y][x]).value_or(0);
            const Status status = weight == 0 ? grid.set_wall(at) : grid.set_weight(at, weight);
            if (status != Status::ok) {
                return failure(status, at);
            }
        }
    }
    return success();
}

} // namespace

MapParse parse_map(std::string_view text, Grid& out) {
    std::vector<std::string_view> rows;
    if (const MapParse result = split_rows(text, rows); result.status != Status::ok) {
        return result;
    }
    Ends ends;
    if (const MapParse result = scan_rows(rows, ends); result.status != Status::ok) {
        return result;
    }
    if (!ends.start) {
        return failure(Status::missing_start, {});
    }
    if (!ends.goal) {
        return failure(Status::missing_goal, {});
    }
    Grid grid;
    if (const MapParse result = build(rows, *ends.start, *ends.goal, grid);
        result.status != Status::ok) {
        return result;
    }
    out = std::move(grid);
    return success();
}

std::string format_map(const Grid& grid) {
    std::string text;
    text.reserve((static_cast<std::size_t>(grid.width()) + 1) *
                 static_cast<std::size_t>(grid.height()));
    for (std::int32_t y = 0; y < grid.height(); ++y) {
        for (std::int32_t x = 0; x < grid.width(); ++x) {
            const Coord at{.x = x, .y = y};
            const bool is_start = at == grid.start();
            const bool is_goal = at == grid.goal();
            if (is_start && is_goal) {
                text.push_back(start_and_goal_symbol);
            } else if (is_start) {
                text.push_back(start_symbol);
            } else if (is_goal) {
                text.push_back(goal_symbol);
            } else if (grid.is_wall(at)) {
                text.push_back(wall_symbol);
            } else if (grid.weight(at) == min_weight) {
                text.push_back(open_symbol);
            } else {
                text.push_back(static_cast<char>('0' + grid.weight(at)));
            }
        }
        text.push_back('\n');
    }
    return text;
}

} // namespace pathfinding
