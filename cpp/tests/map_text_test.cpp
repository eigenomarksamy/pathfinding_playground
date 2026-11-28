#include "pathfinding/grid.hpp"
#include "pathfinding/map_text.hpp"
#include "pathfinding/status.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>

namespace {

using pathfinding::Coord;
using pathfinding::Extent;
using pathfinding::format_map;
using pathfinding::Grid;
using pathfinding::MapParse;
using pathfinding::max_side;
using pathfinding::parse_map;
using pathfinding::Status;

Grid parsed(std::string_view text) {
    Grid grid;
    const MapParse result = parse_map(text, grid);
    EXPECT_EQ(result.status, Status::ok) << "at " << result.at.x << ", " << result.at.y;
    return grid;
}

void expect_error(std::string_view text, Status status, Coord at) {
    Grid grid;
    const Grid before = grid;
    const MapParse result = parse_map(text, grid);
    EXPECT_EQ(result.status, status) << text;
    EXPECT_EQ(result.at, at) << text;
    EXPECT_EQ(grid, before) << "an error leaves the grid unchanged";
}

TEST(MapText, ReadsEverySymbol) {
    const Grid grid = parsed("S.#\n29G\n");
    EXPECT_EQ(grid.extent(), (Extent{3, 2}));
    EXPECT_EQ(grid.start(), (Coord{0, 0}));
    EXPECT_EQ(grid.goal(), (Coord{2, 1}));
    EXPECT_EQ(grid.weight({0, 0}), 1);
    EXPECT_EQ(grid.weight({1, 0}), 1);
    EXPECT_TRUE(grid.is_wall({2, 0}));
    EXPECT_EQ(grid.weight({0, 1}), 2);
    EXPECT_EQ(grid.weight({1, 1}), 9);
    EXPECT_EQ(grid.weight({2, 1}), 1);
}

TEST(MapText, ReadsTheStartAndTheGoalOnOneCell) {
    const Grid grid = parsed("..\n.*\n");
    EXPECT_EQ(grid.start(), (Coord{1, 1}));
    EXPECT_EQ(grid.goal(), (Coord{1, 1}));
}

TEST(MapText, AcceptsCrlfAndAMissingLastLineEnding) {
    const Grid lf = parsed("S.\n.G\n");
    EXPECT_EQ(parsed("S.\r\n.G\r\n"), lf);
    EXPECT_EQ(parsed("S.\n.G"), lf);
    EXPECT_EQ(parsed("S.\r\n.G"), lf);
}

TEST(MapText, WritesWhatItReads) {
    EXPECT_EQ(format_map(parsed("S..#\n.29#\n...G\n")), "S..#\n.29#\n...G\n");
    EXPECT_EQ(format_map(parsed("..\n.*\n")), "..\n.*\n");
    EXPECT_EQ(format_map(parsed("S.\r\n.G")), "S.\n.G\n");
}

TEST(MapText, WritesAnEditedGrid) {
    Grid grid;
    ASSERT_EQ(grid.reset({3, 2}), Status::ok);
    ASSERT_EQ(grid.set_wall({1, 0}), Status::ok);
    ASSERT_EQ(grid.set_weight({1, 1}, 4), Status::ok);
    EXPECT_EQ(format_map(grid), "S#.\n.4G\n");
    EXPECT_EQ(parsed(format_map(grid)), grid);
}

TEST(MapText, WritesAWeightedStartOrGoalAsWeightOne) {
    Grid grid;
    ASSERT_EQ(grid.reset({2, 1}), Status::ok);
    ASSERT_EQ(grid.set_weight(grid.goal(), 5), Status::ok);
    EXPECT_EQ(format_map(grid), "SG\n");
    EXPECT_EQ(parsed(format_map(grid)).weight(grid.goal()), 1);
}

TEST(MapText, ReadsTheLargestGrid) {
    std::string text;
    for (int y = 0; y < max_side; ++y) {
        std::string row(static_cast<std::size_t>(max_side), '.');
        if (y == 0) {
            row.front() = 'S';
        }
        if (y == max_side - 1) {
            row.back() = 'G';
        }
        text += row + '\n';
    }
    const Grid grid = parsed(text);
    EXPECT_EQ(grid.extent(), (Extent{max_side, max_side}));
    EXPECT_EQ(format_map(grid), text);
}

TEST(MapTextErrors, RejectsAnEmptyMapOrAnEmptyFirstRow) {
    expect_error("", Status::invalid_size, {0, 0});
    expect_error("\n", Status::invalid_size, {0, 0});
    expect_error("\nSG\n", Status::invalid_size, {0, 0});
}

TEST(MapTextErrors, RejectsAGridWiderOrTallerThanTheMaximum) {
    std::string wide(static_cast<std::size_t>(max_side) + 1, '.');
    wide.front() = 'S';
    wide.back() = 'G';
    expect_error(wide, Status::invalid_size, {max_side, 0});

    std::string tall = "S\n";
    for (int y = 1; y < max_side; ++y) {
        tall += ".\n";
    }
    tall += "G\n";
    expect_error(tall, Status::invalid_size, {0, max_side});
}

TEST(MapTextErrors, RejectsRaggedAndBlankRows) {
    expect_error("S..\n.G\n", Status::ragged_rows, {2, 1});
    expect_error("S.\n..G\n", Status::ragged_rows, {2, 1});
    expect_error("S.\n\n.G\n", Status::ragged_rows, {0, 1});
    expect_error("S.\n.G\n\n", Status::ragged_rows, {0, 2});
}

TEST(MapTextErrors, RejectsUnknownSymbols) {
    expect_error("S x\n..G\n", Status::unknown_symbol, {1, 0});
    expect_error("S0\n.G\n", Status::unknown_symbol, {1, 0}); // weight 0 isn't terrain
    expect_error("S1\n.G\n", Status::unknown_symbol, {1, 0}); // weight 1 is '.'
    expect_error("S-\n.G\n", Status::unknown_symbol, {1, 0});
    expect_error("S.\n.g\n", Status::unknown_symbol, {1, 1});
    expect_error("S.\r.G\n", Status::unknown_symbol, {2, 0}); // a lone CR isn't a line ending
}

TEST(MapTextErrors, RejectsAMissingOrSecondStartOrGoal) {
    expect_error("..\n.G\n", Status::missing_start, {0, 0});
    expect_error("S.\n..\n", Status::missing_goal, {0, 0});
    expect_error("SS\n.G\n", Status::duplicate_start, {1, 0});
    expect_error("S*\n..\n", Status::duplicate_start, {1, 0});
    expect_error("SG\n.G\n", Status::duplicate_goal, {1, 1});
    expect_error("*G\n..\n", Status::duplicate_goal, {1, 0});
}

} // namespace
