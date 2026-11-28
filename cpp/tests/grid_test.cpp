#include "pathfinding/grid.hpp"
#include "pathfinding/status.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

using pathfinding::Coord;
using pathfinding::diagonal_step_cost;
using pathfinding::Extent;
using pathfinding::fingerprint;
using pathfinding::Grid;
using pathfinding::max_side;
using pathfinding::max_weight;
using pathfinding::Movement;
using pathfinding::Neighbors;
using pathfinding::Status;
using pathfinding::Step;
using pathfinding::straight_step_cost;

std::vector<Step> steps(const Neighbors& neighbors) {
    return {neighbors.begin(), neighbors.end()};
}

Grid open_grid(std::int32_t width, std::int32_t height) {
    Grid grid;
    EXPECT_EQ(grid.reset({width, height}), Status::ok);
    return grid;
}

// a 3 x 3 grid's center, where every neighbor lies inside the grid.
constexpr Coord center{1, 1};

TEST(GridExtent, AcceptsEverySideFromOneToTheMaximum) {
    EXPECT_TRUE(is_valid(Extent{1, 1}));
    EXPECT_TRUE(is_valid(Extent{max_side, max_side}));
    EXPECT_TRUE(is_valid(Extent{1, max_side}));
}

TEST(GridExtent, RejectsEmptyNegativeAndOversizedSides) {
    EXPECT_FALSE(is_valid(Extent{0, 4}));
    EXPECT_FALSE(is_valid(Extent{4, 0}));
    EXPECT_FALSE(is_valid(Extent{-1, 4}));
    EXPECT_FALSE(is_valid(Extent{max_side + 1, 4}));
    EXPECT_FALSE(is_valid(Extent{4, max_side + 1}));
}

TEST(Grid, StartsAsOneOpenCellThatIsTheStartAndTheGoal) {
    const Grid grid;
    EXPECT_EQ(grid.extent(), (Extent{1, 1}));
    EXPECT_EQ(grid.start(), (Coord{0, 0}));
    EXPECT_EQ(grid.goal(), (Coord{0, 0}));
    EXPECT_EQ(grid.weight({0, 0}), 1);
}

TEST(Grid, ResetsToAnOpenGridFromTopLeftToBottomRight) {
    const Grid grid = open_grid(5, 3);
    EXPECT_EQ(grid.cell_count(), 15U);
    EXPECT_EQ(grid.start(), (Coord{0, 0}));
    EXPECT_EQ(grid.goal(), (Coord{4, 2}));
    for (const std::uint8_t cell : grid.cells()) {
        EXPECT_EQ(cell, 1);
    }
}

TEST(Grid, RejectsAnInvalidExtentAndKeepsItself) {
    Grid grid = open_grid(3, 2);
    ASSERT_EQ(grid.set_wall({1, 0}), Status::ok);
    const Grid before = grid;
    EXPECT_EQ(grid.reset({0, 2}), Status::invalid_size);
    EXPECT_EQ(grid.reset({max_side + 1, 2}), Status::invalid_size);
    EXPECT_EQ(grid, before);
}

TEST(Grid, MapsCoordinatesToCellsRowByRow) {
    const Grid grid = open_grid(4, 3);
    EXPECT_EQ(grid.index({0, 0}), 0U);
    EXPECT_EQ(grid.index({3, 0}), 3U);
    EXPECT_EQ(grid.index({0, 1}), 4U);
    EXPECT_EQ(grid.index({3, 2}), 11U);
    for (std::size_t i = 0; i < grid.cell_count(); ++i) {
        EXPECT_EQ(grid.index(grid.coord(i)), i);
    }
}

TEST(Grid, TreatsEverythingOutsideAsWall) {
    const Grid grid = open_grid(3, 3);
    for (const Coord outside : {Coord{-1, 0}, Coord{0, -1}, Coord{3, 0}, Coord{0, 3}}) {
        EXPECT_FALSE(grid.contains(outside));
        EXPECT_TRUE(grid.is_wall(outside));
        EXPECT_EQ(grid.weight(outside), 0);
    }
}

TEST(GridTerrain, StoresWallsAndWeightsInTheirCells) {
    Grid grid = open_grid(3, 1);
    ASSERT_EQ(grid.set_wall({1, 0}), Status::ok);
    ASSERT_EQ(grid.set_weight({2, 0}, 7), Status::ok);
    EXPECT_TRUE(grid.is_wall({1, 0}));
    EXPECT_EQ(grid.weight({1, 0}), 0);
    EXPECT_EQ(grid.weight({2, 0}), 7);
    EXPECT_EQ(std::vector<std::uint8_t>(grid.cells().begin(), grid.cells().end()),
              (std::vector<std::uint8_t>{1, 0, 7}));
}

TEST(GridTerrain, AWeightOpensAWall) {
    Grid grid = open_grid(3, 1);
    ASSERT_EQ(grid.set_wall({1, 0}), Status::ok);
    ASSERT_EQ(grid.set_weight({1, 0}, 1), Status::ok);
    EXPECT_FALSE(grid.is_wall({1, 0}));
}

TEST(GridTerrain, RejectsNegativeZeroAndOversizedWeights) {
    Grid grid = open_grid(3, 1);
    const Grid before = grid;
    for (const std::int32_t weight : {-1, -9, 0, max_weight + 1}) {
        EXPECT_EQ(grid.set_weight({1, 0}, weight), Status::invalid_weight) << weight;
    }
    EXPECT_EQ(grid, before);
}

TEST(GridTerrain, RejectsCellsOutsideTheGrid) {
    Grid grid = open_grid(3, 1);
    EXPECT_EQ(grid.set_wall({3, 0}), Status::out_of_bounds);
    EXPECT_EQ(grid.set_weight({0, 1}, 2), Status::out_of_bounds);
    EXPECT_EQ(grid.set_start({-1, 0}), Status::out_of_bounds);
    EXPECT_EQ(grid.set_goal({0, -1}), Status::out_of_bounds);
}

TEST(GridTerrain, KeepsTheStartAndTheGoalOpen) {
    Grid grid = open_grid(3, 1);
    EXPECT_EQ(grid.set_wall(grid.start()), Status::on_start_or_goal);
    EXPECT_EQ(grid.set_wall(grid.goal()), Status::on_start_or_goal);
    ASSERT_EQ(grid.set_wall({1, 0}), Status::ok);
    EXPECT_EQ(grid.set_start({1, 0}), Status::on_wall);
    EXPECT_EQ(grid.set_goal({1, 0}), Status::on_wall);
    EXPECT_EQ(grid.start(), (Coord{0, 0}));
    EXPECT_EQ(grid.goal(), (Coord{2, 0}));
}

TEST(GridTerrain, MovesTheStartAndTheGoal) {
    Grid grid = open_grid(3, 3);
    ASSERT_EQ(grid.set_start(center), Status::ok);
    ASSERT_EQ(grid.set_goal(center), Status::ok);
    EXPECT_EQ(grid.start(), center);
    EXPECT_EQ(grid.goal(), center);
    // the old corners are free for walls now.
    EXPECT_EQ(grid.set_wall({0, 0}), Status::ok);
    EXPECT_EQ(grid.set_wall({2, 2}), Status::ok);
}

TEST(GridNeighbors, FourWayStepsNorthEastSouthWestAtUnitCost) {
    const Grid grid = open_grid(3, 3);
    const std::vector<Step> expected{
        {{1, 0}, straight_step_cost},
        {{2, 1}, straight_step_cost},
        {{1, 2}, straight_step_cost},
        {{0, 1}, straight_step_cost},
    };
    EXPECT_EQ(steps(grid.neighbors(center, Movement::four_way)), expected);
}

TEST(GridNeighbors, EightWayAddsTheDiagonalsAtTheirCost) {
    const Grid grid = open_grid(3, 3);
    const std::vector<Step> expected{
        {{1, 0}, straight_step_cost}, {{2, 1}, straight_step_cost}, {{1, 2}, straight_step_cost},
        {{0, 1}, straight_step_cost}, {{2, 0}, diagonal_step_cost}, {{2, 2}, diagonal_step_cost},
        {{0, 2}, diagonal_step_cost}, {{0, 0}, diagonal_step_cost},
    };
    EXPECT_EQ(steps(grid.neighbors(center, Movement::eight_way)), expected);
}

TEST(GridNeighbors, StayInsideTheGridAtItsCorners) {
    const Grid grid = open_grid(3, 3);
    EXPECT_EQ(steps(grid.neighbors({0, 0}, Movement::four_way)),
              (std::vector<Step>{{{1, 0}, straight_step_cost}, {{0, 1}, straight_step_cost}}));
    EXPECT_EQ(steps(grid.neighbors({2, 2}, Movement::eight_way)),
              (std::vector<Step>{{{2, 1}, straight_step_cost},
                                 {{1, 2}, straight_step_cost},
                                 {{1, 1}, diagonal_step_cost}}));
}

TEST(GridNeighbors, OneCellHasNone) {
    const Grid grid;
    EXPECT_TRUE(grid.neighbors({0, 0}, Movement::eight_way).empty());
}

TEST(GridNeighbors, CostTheWeightOfTheCellEntered) {
    Grid grid = open_grid(3, 3);
    ASSERT_EQ(grid.set_weight({2, 1}, 5), Status::ok); // east, straight
    ASSERT_EQ(grid.set_weight({2, 0}, 3), Status::ok); // northeast, diagonal
    ASSERT_EQ(grid.set_weight(center, 9), Status::ok); // leaving a cell costs nothing of its own
    const Neighbors neighbors = grid.neighbors(center, Movement::eight_way);
    ASSERT_EQ(neighbors.size(), 8U);
    EXPECT_EQ(neighbors[0], (Step{{1, 0}, straight_step_cost}));
    EXPECT_EQ(neighbors[1], (Step{{2, 1}, 5 * straight_step_cost}));
    EXPECT_EQ(neighbors[4], (Step{{2, 0}, 3 * diagonal_step_cost}));
    // entering the center from the east, the third of north, south and west, costs its weight.
    EXPECT_EQ(grid.neighbors({2, 1}, Movement::four_way)[2],
              (Step{center, 9 * straight_step_cost}));
}

TEST(GridNeighbors, CostTheHeaviestStepWithoutOverflow) {
    Grid grid = open_grid(2, 2);
    ASSERT_EQ(grid.set_weight({1, 1}, max_weight), Status::ok);
    const Neighbors neighbors = grid.neighbors({0, 0}, Movement::eight_way);
    ASSERT_EQ(neighbors.size(), 3U);
    EXPECT_EQ(neighbors[2],
              (Step{{1, 1}, static_cast<std::uint32_t>(max_weight) * diagonal_step_cost}));
}

TEST(GridNeighbors, NeverEnterAWall) {
    Grid grid = open_grid(3, 3);
    ASSERT_EQ(grid.set_wall({2, 1}), Status::ok); // east
    ASSERT_EQ(grid.set_wall({0, 2}), Status::ok); // southwest
    const std::vector<Step> expected{
        {{1, 0}, straight_step_cost},
        {{1, 2}, straight_step_cost},
        {{0, 1}, straight_step_cost},
        {{0, 0}, diagonal_step_cost},
    };
    // northeast and southeast pass the east wall's corners, and southwest is the wall itself.
    EXPECT_EQ(steps(grid.neighbors(center, Movement::eight_way)), expected);
}

TEST(GridNeighbors, AWallHasNoneAndNothingOutsideHasAny) {
    Grid grid = open_grid(3, 3);
    ASSERT_EQ(grid.set_wall(center), Status::ok);
    EXPECT_TRUE(grid.neighbors(center, Movement::eight_way).empty());
    EXPECT_TRUE(grid.neighbors({3, 1}, Movement::eight_way).empty());
    EXPECT_TRUE(grid.neighbors({-1, -1}, Movement::four_way).empty());
}

TEST(GridCornerCutting, OneOrthogonalWallBlocksTheDiagonalsBesideIt) {
    Grid grid = open_grid(3, 3);
    ASSERT_EQ(grid.set_wall({1, 0}), Status::ok); // north
    const std::vector<Step> expected{
        {{2, 1}, straight_step_cost}, {{1, 2}, straight_step_cost}, {{0, 1}, straight_step_cost},
        {{2, 2}, diagonal_step_cost}, {{0, 2}, diagonal_step_cost},
    };
    // northeast and northwest would cut the north wall's corners.
    EXPECT_EQ(steps(grid.neighbors(center, Movement::eight_way)), expected);
}

TEST(GridCornerCutting, NoSqueezingBetweenWallsThatTouchAtACorner) {
    Grid grid = open_grid(2, 2);
    ASSERT_EQ(grid.set_wall({1, 0}), Status::ok);
    ASSERT_EQ(grid.set_wall({0, 1}), Status::ok);
    EXPECT_TRUE(grid.neighbors({0, 0}, Movement::eight_way).empty());
    EXPECT_TRUE(grid.neighbors({1, 1}, Movement::eight_way).empty());
}

TEST(GridCornerCutting, AppliesBothWays) {
    Grid grid = open_grid(2, 2);
    ASSERT_EQ(grid.set_wall({1, 0}), Status::ok);
    // (0, 0) to (1, 1) and back pass the same corner.
    EXPECT_EQ(steps(grid.neighbors({0, 0}, Movement::eight_way)),
              (std::vector<Step>{{{0, 1}, straight_step_cost}}));
    EXPECT_EQ(steps(grid.neighbors({1, 1}, Movement::eight_way)),
              (std::vector<Step>{{{0, 1}, straight_step_cost}}));
}

TEST(GridCornerCutting, WeightedTerrainBesideADiagonalDoesNotBlockIt) {
    Grid grid = open_grid(2, 2);
    ASSERT_EQ(grid.set_weight({1, 0}, max_weight), Status::ok);
    ASSERT_EQ(grid.set_weight({0, 1}, max_weight), Status::ok);
    const Neighbors neighbors = grid.neighbors({0, 0}, Movement::eight_way);
    ASSERT_EQ(neighbors.size(), 3U);
    EXPECT_EQ(neighbors[2], (Step{{1, 1}, diagonal_step_cost}));
}

TEST(GridCornerCutting, DoesNotApplyToFourWayMoves) {
    Grid grid = open_grid(3, 3);
    ASSERT_EQ(grid.set_wall({2, 0}), Status::ok); // northeast
    EXPECT_EQ(grid.neighbors(center, Movement::four_way).size(), 4U);
}

TEST(GridFingerprint, IsEqualForEqualGrids) {
    EXPECT_EQ(fingerprint(open_grid(4, 3)), fingerprint(open_grid(4, 3)));
}

TEST(GridFingerprint, ChangesWithEveryPartOfTheGrid) {
    const Grid base = open_grid(4, 3);
    const std::uint64_t original = fingerprint(base);
    EXPECT_NE(fingerprint(open_grid(3, 4)), original);
    Grid walled = base;
    ASSERT_EQ(walled.set_wall({1, 1}), Status::ok);
    EXPECT_NE(fingerprint(walled), original);
    Grid weighted = base;
    ASSERT_EQ(weighted.set_weight({1, 1}, 2), Status::ok);
    EXPECT_NE(fingerprint(weighted), original);
    Grid moved_start = base;
    ASSERT_EQ(moved_start.set_start({1, 0}), Status::ok);
    EXPECT_NE(fingerprint(moved_start), original);
    Grid moved_goal = base;
    ASSERT_EQ(moved_goal.set_goal({2, 2}), Status::ok);
    EXPECT_NE(fingerprint(moved_goal), original);
}

TEST(GridFingerprint, IsPinnedForAnOpenGrid) {
    // FNV-1a over the extent, the start, the goal and the cells: the same on every platform.
    // computed apart from the engine, from FNV-1a's definition.
    EXPECT_EQ(fingerprint(Grid{}), 0x45129f1ce178881cULL);
}

} // namespace
