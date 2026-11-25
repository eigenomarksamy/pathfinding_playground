# the engine: C++20 with no rendering, browser or framework code, built natively and to WebAssembly from these sources.
add_library(pathfinding_engine STATIC
  src/probe.cpp
  src/version.cpp)
add_library(pathfinding::engine ALIAS pathfinding_engine)
target_include_directories(pathfinding_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(pathfinding_engine PUBLIC cxx_std_20)
target_compile_definitions(pathfinding_engine PRIVATE PATHFINDING_VERSION="${PROJECT_VERSION}")
target_link_libraries(pathfinding_engine PRIVATE pathfinding_warnings)
