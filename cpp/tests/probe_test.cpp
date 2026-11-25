#include "pathfinding/probe.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace {

using pathfinding::probe::band_period;
using pathfinding::probe::cell_count;
using pathfinding::probe::fill_bands;
using pathfinding::probe::is_valid;
using pathfinding::probe::max_side;
using pathfinding::probe::Status;

TEST(ProbeExtent, AcceptsEverySideFromOneToTheMaximum) {
    EXPECT_TRUE(is_valid({1, 1}));
    EXPECT_TRUE(is_valid({max_side, max_side}));
    EXPECT_TRUE(is_valid({1, max_side}));
}

TEST(ProbeExtent, RejectsEmptyNegativeAndOversizedSides) {
    EXPECT_FALSE(is_valid({0, 4}));
    EXPECT_FALSE(is_valid({4, 0}));
    EXPECT_FALSE(is_valid({-1, 4}));
    EXPECT_FALSE(is_valid({max_side + 1, 4}));
    EXPECT_FALSE(is_valid({4, max_side + 1}));
}

TEST(ProbeExtent, CountsCellsOnlyForValidExtents) {
    EXPECT_EQ(cell_count({3, 2}), 6U);
    EXPECT_EQ(cell_count({max_side, max_side}), 65536U);
    EXPECT_EQ(cell_count({0, 2}), 0U);
}

TEST(ProbeBands, FillsTheKnownPatternRowByRow) {
    std::array<std::uint8_t, 12> cells{};
    ASSERT_EQ(fill_bands({4, 3}, 0, cells), Status::ok);
    // lit where (x + y) mod 8 < 3
    const std::array<std::uint8_t, 12> expected{
        1, 1, 1, 0, // y = 0
        1, 1, 0, 0, // y = 1
        1, 0, 0, 0, // y = 2
    };
    EXPECT_EQ(cells, expected);
}

TEST(ProbeBands, EachStepMovesTheBandsOneDiagonal) {
    std::array<std::uint8_t, 8> step0{};
    std::array<std::uint8_t, 8> step1{};
    ASSERT_EQ(fill_bands({8, 1}, 0, step0), Status::ok);
    ASSERT_EQ(fill_bands({8, 1}, 1, step1), Status::ok);
    for (std::size_t x = 1; x < step0.size(); ++x) {
        EXPECT_EQ(step1.at(x - 1), step0.at(x)) << "x = " << x;
    }
}

TEST(ProbeBands, RepeatsEveryPeriod) {
    std::vector<std::uint8_t> first(cell_count({16, 16}));
    std::vector<std::uint8_t> later(first.size());
    ASSERT_EQ(fill_bands({16, 16}, 5, first), Status::ok);
    ASSERT_EQ(fill_bands({16, 16}, 5 + (3 * band_period), later), Status::ok);
    EXPECT_EQ(first, later);
}

TEST(ProbeBands, HandlesTheLargestPhase) {
    std::array<std::uint8_t, 4> cells{};
    EXPECT_EQ(fill_bands({4, 1}, std::numeric_limits<std::uint32_t>::max(), cells), Status::ok);
}

TEST(ProbeBands, RejectsAnInvalidExtentWithoutWriting) {
    std::array<std::uint8_t, 4> cells{7, 7, 7, 7};
    EXPECT_EQ(fill_bands({0, 4}, 0, cells), Status::invalid_size);
    EXPECT_EQ(fill_bands({max_side + 1, 1}, 0, cells), Status::invalid_size);
    EXPECT_EQ(cells, (std::array<std::uint8_t, 4>{7, 7, 7, 7}));
}

TEST(ProbeBands, RejectsABufferTooSmallWithoutWriting) {
    std::array<std::uint8_t, 5> cells{7, 7, 7, 7, 7};
    EXPECT_EQ(fill_bands({3, 2}, 0, cells), Status::buffer_too_small);
    EXPECT_EQ(cells, (std::array<std::uint8_t, 5>{7, 7, 7, 7, 7}));
}

TEST(ProbeBands, LeavesCellsPastTheExtentAlone) {
    std::array<std::uint8_t, 3> cells{7, 7, 7};
    ASSERT_EQ(fill_bands({2, 1}, 0, cells), Status::ok);
    EXPECT_EQ(cells.at(2), 7);
}

} // namespace
