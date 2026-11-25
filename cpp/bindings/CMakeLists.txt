# the WebAssembly module: the engine and its Embind interface, as an ES module the web app imports.
add_executable(pathfinding_wasm engine_bindings.cpp)
set_target_properties(pathfinding_wasm PROPERTIES OUTPUT_NAME pathfinding SUFFIX ".js")
target_link_libraries(pathfinding_wasm PRIVATE pathfinding::engine pathfinding_warnings)

set(pathfinding_tsd ${CMAKE_CURRENT_BINARY_DIR}/pathfinding.d.ts)
target_link_options(pathfinding_wasm PRIVATE
  -lembind
  # TypeScript declarations of the Embind interface, which the web app's adapter imports
  "--emit-tsd=${pathfinding_tsd}"
  -sMODULARIZE=1
  -sEXPORT_ES6=1
  -sEXPORT_NAME=createPathfindingModule
  # the browser, and Node for the adapter's tests
  -sENVIRONMENT=web,node
  -sFILESYSTEM=0
  # no eval or new Function, so the page's content security policy needs no 'unsafe-eval'
  -sDYNAMIC_EXECUTION=0)

# `npm run wasm` installs these into web/src/engine/wasm/.
install(FILES
  $<TARGET_FILE:pathfinding_wasm>
  $<TARGET_FILE_DIR:pathfinding_wasm>/pathfinding.wasm
  ${pathfinding_tsd}
  DESTINATION .)
