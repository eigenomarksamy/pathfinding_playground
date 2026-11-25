add_executable(pathfinding_benchmark probe_benchmark.cpp)
target_link_libraries(pathfinding_benchmark PRIVATE pathfinding::engine pathfinding_warnings)

# a short run under CTest keeps the benchmark building and running. timings come from a run by hand.
if(BUILD_TESTING)
  add_test(NAME benchmark.smoke COMMAND pathfinding_benchmark --iterations 10)
endif()
