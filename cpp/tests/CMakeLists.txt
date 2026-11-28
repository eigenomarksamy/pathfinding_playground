include(FetchContent)

# GoogleTest at a pinned release, checked by its hash.
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.tar.gz
  URL_HASH SHA256=6e3191c1455468b3fc35a417fb565c1c5071aee1b7e7f85e30cf48a98d37d8b5
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SYSTEM)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(pathfinding_tests
  canonical_maps_test.cpp
  fixtures.cpp
  generate_test.cpp
  grid_test.cpp
  map_text_test.cpp
  probe_test.cpp
  status_test.cpp
  version_test.cpp)
target_link_libraries(pathfinding_tests PRIVATE pathfinding::engine pathfinding_warnings GTest::gtest_main)
# the canonical test maps, read from the source tree, so no copy can drift from them.
target_compile_definitions(pathfinding_tests PRIVATE
  PATHFINDING_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/maps")

include(GoogleTest)
gtest_discover_tests(pathfinding_tests DISCOVERY_MODE PRE_TEST)
