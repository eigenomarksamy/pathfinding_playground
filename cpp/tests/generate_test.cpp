#include "pathfinding/generate.hpp"
#include "pathfinding/grid.hpp"
#include "pathfinding/status.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>

namespace {

using pathfinding::Coord;
using pathfinding::Extent;
using pathfinding::fingerprint;
using pathfinding::generate;
using pathfinding::Generation;
using pathfinding::Grid;
using pathfinding::max_percent;
using pathfinding::max_side;
using pathfinding::max_weight;
using pathfinding::SplitMix64;
using pathfinding::Status;

Grid generated(const Generation& options) {
    Grid grid;
    EXPECT_EQ(generate(options, grid), Status::ok);
    return grid;
}

constexpr Generation mixed{
    .extent = {40, 30}, .seed = 7, .wall_percent = 30, .weighted_percent = 20};

TEST(SplitMix64, MatchesTheReferenceSequence) {
    // the first outputs for seed 0, from the published algorithm.
    SplitMix64 random(0);
    EXPECT_EQ(random.next(), 0xe220a8397b1dcdafULL);
    EXPECT_EQ(random.next(), 0x6e789e6aa1b965f4ULL);
    EXPECT_EQ(random.next(), 0x06c45d188009454fULL);
}

TEST(SplitMix64, DrawsStayBelowTheirBound) {
    SplitMix64 random(42);
    std::array<std::uint32_t, 7> counts{};
    for (int i = 0; i < 7000; ++i) {
        const std::uint32_t draw = random.below(7);
        ASSERT_LT(draw, 7U);
        ++counts.at(draw);
    }
    for (const std::uint32_t count : counts) {
        EXPECT_GT(count, 800U); // about 1000 each
    }
    EXPECT_EQ(random.below(1), 0U);
    EXPECT_EQ(random.below(0), 0U);
}

TEST(Generate, TheSameSeedGivesTheSameMapTwice) {
    const Grid first = generated(mixed);
    const Grid second = generated(mixed);
    EXPECT_EQ(first, second);
    EXPECT_EQ(fingerprint(first), fingerprint(second));
}

TEST(Generate, AnotherSeedGivesAnotherMap) {
    Generation other = mixed;
    other.seed = mixed.seed + 1;
    EXPECT_NE(generated(other), generated(mixed));
}

TEST(Generate, IsPinnedForAFixedSeed) {
    // the same on every platform and build: the standard library's distributions aren't used.
    // checked against an implementation apart from the engine, from generate.hpp's description.
    EXPECT_EQ(fingerprint(generated(mixed)), 0x0124bc26d7e3e8b1ULL);
}

TEST(Generate, KeepsTheStartAndTheGoalOpenAtTheCorners) {
    Generation all_walls = mixed;
    all_walls.wall_percent = max_percent;
    all_walls.weighted_percent = 0;
    const Grid grid = generated(all_walls);
    EXPECT_EQ(grid.start(), (Coord{0, 0}));
    EXPECT_EQ(grid.goal(), (Coord{39, 29}));
    for (std::size_t i = 0; i < grid.cell_count(); ++i) {
        const Coord at = grid.coord(i);
        if (at == grid.start() || at == grid.goal()) {
            EXPECT_EQ(grid.weight(at), 1);
        } else {
            EXPECT_TRUE(grid.is_wall(at)) << at.x << ", " << at.y;
        }
    }
}

TEST(Generate, NoWallsOrWeightsGivesAnOpenGrid) {
    const Grid grid = generated({.extent = {12, 9}, .seed = 99});
    Grid open;
    ASSERT_EQ(open.reset({12, 9}), Status::ok);
    EXPECT_EQ(grid, open);
}

TEST(Generate, WeightedTerrainRunsFromTwoToTheMaximum) {
    const Grid grid = generated({.extent = {64, 64}, .seed = 3, .weighted_percent = max_percent});
    std::array<int, max_weight + 1> seen{};
    for (std::size_t i = 0; i < grid.cell_count(); ++i) {
        const Coord at = grid.coord(i);
        ASSERT_FALSE(grid.is_wall(at));
        ++seen.at(static_cast<std::size_t>(grid.weight(at)));
    }
    EXPECT_EQ(seen.at(1), 2); // the start and the goal
    for (std::size_t weight = 2; weight <= static_cast<std::size_t>(max_weight); ++weight) {
        EXPECT_GT(seen.at(weight), 0) << weight;
    }
}

TEST(Generate, KeepsRoughlyTheAskedShareOfWalls) {
    const Grid grid = generated(mixed);
    std::size_t walls = 0;
    for (const std::uint8_t cell : grid.cells()) {
        walls += cell == 0 ? 1U : 0U;
    }
    // 30% of 1198 cells is about 359.
    EXPECT_GT(walls, 300U);
    EXPECT_LT(walls, 420U);
}

TEST(Generate, AOneCellMapIsTheStartAndTheGoal) {
    const Grid grid = generated({.extent = {1, 1}, .seed = 5, .wall_percent = max_percent});
    EXPECT_EQ(grid, Grid{});
}

TEST(Generate, RejectsAnInvalidExtentOrPercentageAndKeepsTheGrid) {
    Grid grid = generated(mixed);
    const Grid before = grid;
    EXPECT_EQ(generate({.extent = {0, 5}}, grid), Status::invalid_size);
    EXPECT_EQ(generate({.extent = {max_side + 1, 5}}, grid), Status::invalid_size);
    EXPECT_EQ(generate({.extent = {5, 5}, .wall_percent = -1}, grid), Status::invalid_option);
    EXPECT_EQ(generate({.extent = {5, 5}, .wall_percent = 101}, grid), Status::invalid_option);
    EXPECT_EQ(generate({.extent = {5, 5}, .weighted_percent = -1}, grid), Status::invalid_option);
    EXPECT_EQ(generate({.extent = {5, 5}, .wall_percent = 60, .weighted_percent = 41}, grid),
              Status::invalid_option);
    EXPECT_EQ(grid, before);
}

TEST(Generate, AcceptsPercentagesThatSumToTheMaximum) {
    Grid grid;
    EXPECT_EQ(generate({.extent = {5, 5}, .wall_percent = 60, .weighted_percent = 40}, grid),
              Status::ok);
    EXPECT_EQ(grid.extent(), (Extent{5, 5}));
}

} // namespace
