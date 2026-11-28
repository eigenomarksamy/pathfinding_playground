#include "pathfinding/status.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <string_view>

namespace {

using pathfinding::Status;

TEST(Status, EveryValueHasItsOwnName) {
    std::set<std::string_view> names;
    for (std::uint8_t value = 0; value <= static_cast<std::uint8_t>(Status::duplicate_goal);
         ++value) {
        const std::string_view name = pathfinding::to_string(static_cast<Status>(value));
        EXPECT_NE(name, "unknown") << int{value};
        EXPECT_TRUE(names.insert(name).second) << name;
    }
    EXPECT_EQ(pathfinding::to_string(Status::ok), "ok");
    EXPECT_EQ(pathfinding::to_string(Status::invalid_weight), "invalid_weight");
}

} // namespace
