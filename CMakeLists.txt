cmake_minimum_required(VERSION 3.25)

project(pathfinding_playground
  VERSION 0.1.0
  DESCRIPTION "an interactive laboratory for graph search"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PATHFINDING_WARNINGS_AS_ERRORS "treat the engine's compiler warnings as errors" ON)
option(PATHFINDING_SANITIZE "build with AddressSanitizer and UndefinedBehaviorSanitizer (GCC or Clang)" OFF)

# the sanitizers apply to everything, GoogleTest included, so its code and ours link alike.
if(PATHFINDING_SANITIZE)
  if(MSVC)
    message(FATAL_ERROR "PATHFINDING_SANITIZE is for GCC and Clang: use the linux-sanitize preset")
  endif()
  add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# the engine's own warnings. third-party code doesn't get them.
add_library(pathfinding_warnings INTERFACE)
if(MSVC)
  target_compile_options(pathfinding_warnings INTERFACE /W4 /permissive- /utf-8)
  if(PATHFINDING_WARNINGS_AS_ERRORS)
    target_compile_options(pathfinding_warnings INTERFACE /WX)
  endif()
else()
  target_compile_options(pathfinding_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wold-style-cast)
  if(PATHFINDING_WARNINGS_AS_ERRORS)
    target_compile_options(pathfinding_warnings INTERFACE -Werror)
  endif()
endif()

add_subdirectory(cpp)

# the native build has the tests and the benchmark. the WebAssembly build has the module.
if(EMSCRIPTEN)
  add_subdirectory(cpp/bindings)
else()
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(cpp/tests)
  endif()
  add_subdirectory(benchmarks)
endif()
