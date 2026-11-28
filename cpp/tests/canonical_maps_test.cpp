#include "pathfinding/generate.hpp"
#include "pathfinding/grid.hpp"
#include "pathfinding/map_text.hpp"
#include "pathfinding/status.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include "fixtures.hpp"

namespace {

namespace fixtures = pathfinding::fixtures;
using pathfinding::Coord;
using pathfinding::Extent;
using pathfinding::fingerprint;
using pathfinding::format_map;
using pathfinding::generate;
using pathfinding::Grid;
using pathfinding::MapParse;
using pathfinding::max_side;
using pathfinding::Movement;
using pathfinding::parse_map;
using pathfinding::Status;

Grid load(std::string_view name) {
    const std::optional<std::string> text = fixtures::read_map_text(name);
    EXPECT_TRUE(text.has_value()) << "can't read the map " << name;
    Grid grid;
    if (text) {
        const MapParse result = parse_map(*text, grid);
        EXPECT_EQ(result.status, Status::ok)
            << name << ": " << pathfinding::to_string(result.status) << " at " << result.at.x
            << ", " << result.at.y;
    }
    return grid;
}

class CanonicalMap : public testing::TestWithParam<std::string_view> {};

TEST_P(CanonicalMap, LoadsWithoutErrorAndWritesBackItsOwnText) {
    const std::optional<std::string> text = fixtures::read_map_text(GetParam());
    ASSERT_TRUE(text.has_value()) << "can't read the map " << GetParam();
    Grid grid;
    const MapParse result = parse_map(*text, grid);
    ASSERT_EQ(result.status, Status::ok)
        << pathfinding::to_string(result.status) << " at " << result.at.x << ", " << result.at.y;
    // the files are LF only, so the round trip is exact.
    EXPECT_EQ(format_map(grid), *text);
    EXPECT_FALSE(grid.is_wall(grid.start()));
    EXPECT_FALSE(grid.is_wall(grid.goal()));
}

INSTANTIATE_TEST_SUITE_P(Maps, CanonicalMap, testing::ValuesIn(fixtures::map_names),
                         // GoogleTest's macro already has a parameter named `info`.
                         [](const testing::TestParamInfo<std::string_view>& map) {
                             return std::string(map.param);
                         });

TEST(CanonicalMaps, OpenHasNoWallsOrWeights) {
    const Grid grid = load("open");
    EXPECT_EQ(grid.extent(), (Extent{8, 5}));
    for (const std::uint8_t cell : grid.cells()) {
        EXPECT_EQ(cell, 1);
    }
}

TEST(CanonicalMaps, WallsHasOneGap) {
    const Grid grid = load("walls");
    EXPECT_FALSE(grid.is_wall({4, 3}));
    for (const std::int32_t y : {0, 1, 2, 4}) {
        EXPECT_TRUE(grid.is_wall({4, y})) << y;
    }
}

TEST(CanonicalMaps, WeightedHasHeavyTerrainAcrossTheStraightRoute) {
    const Grid grid = load("weighted");
    EXPECT_EQ(grid.weight({3, 0}), 2);
    EXPECT_EQ(grid.weight({4, 0}), 3);
    EXPECT_EQ(grid.weight({5, 0}), 4);
    for (std::int32_t y = 1; y <= 3; ++y) {
        for (std::int32_t x = 3; x <= 5; ++x) {
            EXPECT_EQ(grid.weight({x, y}), 9) << x << ", " << y;
        }
    }
    EXPECT_EQ(grid.weight({4, 4}), 1); // the open detour
}

TEST(CanonicalMaps, StartIsGoalHasThemOnOneCell) {
    const Grid grid = load("start_is_goal");
    EXPECT_EQ(grid.start(), (Coord{1, 1}));
    EXPECT_EQ(grid.goal(), grid.start());
}

TEST(CanonicalMaps, NoPathWallsTheGoalIn) {
    const Grid grid = load("no_path");
    for (const Movement movement : {Movement::four_way, Movement::eight_way}) {
        EXPECT_TRUE(grid.neighbors(grid.goal(), movement).empty());
    }
}

TEST(CanonicalMaps, BlockedOnlyLetsCornerCuttingCrossItsDiagonal) {
    const Grid grid = load("blocked");
    // (2, 2) to (3, 3) passes between the walls at (3, 2) and (2, 3), which touch at a corner.
    ASSERT_TRUE(grid.is_wall({3, 2}));
    ASSERT_TRUE(grid.is_wall({2, 3}));
    for (const auto& step : grid.neighbors({2, 2}, Movement::eight_way)) {
        EXPECT_NE(step.to, (Coord{3, 3}));
    }
    // the way round is open, down the left side.
    EXPECT_FALSE(grid.is_wall({0, 5}));
}

TEST(CanonicalMaps, TinyIsTwoCells) {
    const Grid grid = load("tiny");
    EXPECT_EQ(grid.extent(), (Extent{2, 1}));
    EXPECT_EQ(grid.neighbors(grid.start(), Movement::eight_way).size(), 1U);
}

TEST(CanonicalMaps, LargeIsGeneratedWithoutErrorAtTheLargestSize) {
    Grid grid;
    ASSERT_EQ(generate(fixtures::large_map, grid), Status::ok);
    EXPECT_EQ(grid.extent(), (Extent{max_side, max_side}));
    EXPECT_FALSE(grid.is_wall(grid.start()));
    EXPECT_FALSE(grid.is_wall(grid.goal()));
    EXPECT_EQ(fingerprint(grid), fixtures::large_map_fingerprint);
    // and its text reads back as the same grid.
    Grid reread;
    ASSERT_EQ(parse_map(format_map(grid), reread).status, Status::ok);
    EXPECT_EQ(reread, grid);
}

} // namespace
