#include "pathfinding/version.hpp"

#include <gtest/gtest.h>

TEST(EngineVersion, IsTheProjectVersion) {
    EXPECT_EQ(pathfinding::engine_version(), "0.1.0");
}
