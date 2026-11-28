#ifndef PATHFINDING_MAP_TEXT_HPP
#define PATHFINDING_MAP_TEXT_HPP

#include "pathfinding/grid.hpp"
#include "pathfinding/status.hpp"

#include <string>
#include <string_view>

// a grid as text, the format of the canonical test maps:
//
//   - one line per row, the top row first, every row as wide as the first. the width and the
//     height run from 1 to max_side.
//   - '.' is an open cell of weight 1, '2' to '9' an open cell of that weight, '#' a wall,
//     'S' the start, 'G' the goal, and '*' the start and the goal on one cell. 'S', 'G' and '*'
//     are open cells of weight 1, so the format can't yet give the start or the goal another
//     weight: format_map writes them as 1.
//   - lines end in LF or CRLF, and the last line's ending is optional. nothing else is allowed:
//     no blank lines, spaces or comments.
namespace pathfinding {

struct MapParse {
    Status status = Status::ok;
    Coord at; // the column and row where the error was found, where one applies
};

// reads `text` into `out`, which is left unchanged unless the status is ok.
[[nodiscard]] MapParse parse_map(std::string_view text, Grid& out);

// the grid as map text, one LF after each row.
[[nodiscard]] std::string format_map(const Grid& grid);

} // namespace pathfinding

#endif // PATHFINDING_MAP_TEXT_HPP
