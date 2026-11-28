# pathfinding playground architecture

how the playground is laid out, built, tested and deployed. the plan, [docs/plan.md](plan.md), says what is built next, and the outcomes of shared decision SD3 it lists bind everything here.

## layers

```text
browser
├── React and TypeScript app            web/src/
│   ├── components: controls, panels    web/src/components/
│   ├── fixed-step simulation clock     web/src/simulation/
│   └── Canvas 2D drawing               web/src/visualization/
├── typed Wasm adapter                  web/src/engine/engine.ts
└── WebAssembly module                  built from cpp/bindings/ into web/src/engine/wasm/ (not committed)
    └── C++20 engine                    cpp/include/, cpp/src/

native tests and benchmark              cpp/tests/, benchmarks/
└── the same C++20 engine
```

- **the engine** (`cpp/`) is a static library, `pathfinding_engine`, with no rendering, browser or framework code. both builds compile the same sources. it holds the [grid](#the-grid) every algorithm searches, its map text and its seeded maps, from M7, and the bootstrap's probe, `pathfinding::probe`: a pattern of diagonal bands that moves one cell per step, which tests the builds, the bounds checks and the transfer of a frame. the app draws the probe until M9's interface hands it the grid.
- **the boundary** (`cpp/bindings/engine_bindings.cpp`) is Embind, and stays small and batched: a call returns a whole frame as one `Uint8Array`, copied into memory JavaScript owns, never one call per cell. the engine returns status codes, not exceptions, so the module is built without C++ exception support. `--emit-tsd` generates `pathfinding.d.ts`, the module's TypeScript declarations, at build time.
- **the adapter** (`web/src/engine/engine.ts`) is the only code that imports the module. it loads it, checks the app's arguments, and turns the engine's statuses and any failure into an `EngineError` with a code: `load_failed`, `invalid_input`, `invalid_size` or `unexpected`.
- **the app** steps the simulation on a fixed clock, apart from the display's frame rate, and caps the steps per frame, so a hidden tab can't flood the engine. with reduced motion asked for, it starts paused.

## the grid

one grid graph and one cost definition, which every algorithm uses, so their results compare. it is `cpp/include/pathfinding/grid.hpp`, from M7.

- **cells:** a rectangular grid, 1 to 256 cells a side, stored row by row as one byte a cell: 0 for a wall, and otherwise the cell's terrain weight, 1 to 9. x runs right and y down, from (0, 0) at the top left. anything outside the grid counts as a wall.
- **the start and the goal** are always open cells inside the grid: a wall can't be placed on either, and neither can be moved onto a wall. they may be one cell.
- **weights** are kept apart from walls, so the grid never holds a weight below 1: a negative or zero weight is rejected with `invalid_weight` before any search can see it.
- **costs:** a step costs the weight of the cell it enters, times 1000 for a straight step or 1414 for a diagonal, √2 rounded down. costs are integers, so every build, native or WebAssembly, sums and compares them exactly alike, and ties break the same way. with the same two constants, Manhattan and octile distances never overestimate, so A*'s heuristics stay admissible and consistent (M8).
- **movement:** 4-way steps north, east, south and west. 8-way adds the diagonals, under the no-corner-cutting rule: a diagonal step is allowed only when both orthogonal cells beside it are open, so a path never slips past a wall's corner or between two walls that touch at a corner. weighted terrain beside a diagonal doesn't block it.
- **neighbor order:** north, east, south, west, then northeast, southeast, southwest and northwest. searches expand them in this order, so a trace is the same on every build.
- **statuses:** the grid, the map text and the generator return a `pathfinding::Status`, never an exception, and leave their output unchanged unless it is `ok`. the values are stable, since M9 hands them to the app.
- **map text** (`map_text.hpp`): one line per row, `.` for weight 1, `2` to `9` for heavier terrain, `#` for a wall, `S` for the start, `G` for the goal and `*` for both on one cell, with LF or CRLF line endings. a parse error names its status and the cell where it was found. the start and the goal are written as weight 1, so the format can't yet give either another weight: L17's import and export can extend it.
- **seeded maps** (`generate.hpp`): a map from an extent, a seed, a wall percentage and a weighted percentage, the same on every platform. the standard library's distributions differ between implementations, so the generator is SplitMix64 with its own unbiased bounded draw. every cell takes two draws, row by row, so changing one cell's terrain never shifts another's. a path between the start and the goal isn't guaranteed: maze generation is L17's.
- **fingerprints:** `fingerprint(grid)` is a 64-bit FNV-1a hash of the extent, the start, the goal and every cell, read byte by byte, so it is the same on every platform. the tests pin the fingerprints of seeded maps, each checked against a Python version of the generator that doesn't use the engine.

### canonical test maps

the maps every algorithm is tested on, in `cpp/tests/fixtures/`. the native tests read them from the source tree, and as files they can serve the WebAssembly build's tests too (M9).

| map | what it checks |
| --- | --- |
| `open` | 8 × 5, no walls, weight 1 everywhere |
| `walls` | a wall with one gap |
| `weighted` | heavy terrain, weights 2 to 9, across the straight route, with an open detour |
| `start_is_goal` | the start and the goal on one cell |
| `no_path` | the goal walled in on every side |
| `blocked` | a diagonal line of walls that only corner cutting could cross, with a way round |
| `tiny` | 2 × 1 |
| `large` | 256 × 256, generated from a fixed seed with 25% walls and 15% weighted terrain, its fingerprint pinned, so no map of that size is committed |

## layout

the layout is SD3's suggested one: `cpp/` (`include/`, `src/`, `bindings/`, `tests/`), `benchmarks/`, `web/`, `docs/`, `scripts/`, `CMakeLists.txt`, `CMakePresets.json`, `wrangler.jsonc` and `.github/workflows/`, beside `CLAUDE.md`, `project.yaml`, `README.md` and `.gitattributes`.

## builds

`CMakePresets.json` holds every build. each `cmake --workflow --preset <name>` configures, builds and, for the native presets, runs the tests. each build goes to `build/<preset>/`.

| preset | where | what |
| --- | --- | --- |
| `windows-msvc` | Windows | Visual Studio 2022's generator and MSVC, `RelWithDebInfo` |
| `linux-gcc` | Linux | Ninja and GCC, `RelWithDebInfo` |
| `linux-clang` | Linux | Ninja and Clang, `RelWithDebInfo`. its compile database is clang-tidy's |
| `linux-sanitize` | Linux | Clang with AddressSanitizer and UndefinedBehaviorSanitizer, `Debug` |
| `wasm` | Windows or Linux | Ninja and Emscripten from `emsdk`, through `$EMSDK`'s toolchain file, `Release` |

- the engine's code builds with warnings as errors: `/W4 /permissive-` on MSVC, and `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wold-style-cast` on GCC and Clang. `-DPATHFINDING_WARNINGS_AS_ERRORS=OFF` turns the errors off for a local experiment.
- GoogleTest comes through CMake's `FetchContent`, from a pinned release checked by its hash. it is the only C++ dependency. the benchmark is a plain `std::chrono` executable, and CTest runs it once, briefly, so it keeps building.
- `npm run wasm`, in `web/`, runs the `wasm` workflow and installs `pathfinding.js`, `pathfinding.wasm` and `pathfinding.d.ts` into `web/src/engine/wasm/`, where Vite and Vitest import them. the module is an ES module built for the browser and for Node, so the adapter's tests run against the real module. Vite's build therefore notes that the module's Node branch imports `node:module`, which the browser never runs.
- the helper scripts are Python (`scripts/lint.py`) or `package.json` scripts. none is a shell script, and nothing needs WSL.

## checks

- **native:** GoogleTest under CTest, on MSVC locally, and on GCC, Clang, and Clang with the sanitizers in CI. the grid's tests cover 4-way and 8-way neighbors, their costs, weighted terrain and the no-corner-cutting rule. the generator's tests generate maps twice from one seed, and pin fingerprints. every canonical map must load and write back its own text.
- **lint:** `uv run scripts/lint.py` checks the C++ format with clang-format, and runs clang-tidy over the engine and the benchmark with `.clang-tidy`'s checks, as errors. both tools come pinned from PyPI, so Windows and CI run the same versions. clang-tidy needs a Ninja build's compile database, so on Windows only the format is checked by default.
- **web:** `npm run check` runs the TypeScript compiler, Prettier's check and Vitest. the adapter's tests load the real WebAssembly module.

## delivery

`.github/workflows/ci.yml` runs on every push and pull request, on `ubuntu-latest`: `native` (a job for each Linux preset), `lint`, and `web`, which builds the module with Emscripten 7.0.1, checks and builds the app, and keeps `web/dist` as an artifact. on a push to `main`, once all of them pass, `deploy` publishes that artifact to Cloudflare Workers static assets with Wrangler, at `paths.eigenomar.com` (`wrangler.jsonc`). Cloudflare's own build isn't used, since it can't run Emscripten. `web/public/_headers` sets the content security policy, which allows only the app's own scripts plus `'wasm-unsafe-eval'` for the module, which is built with `-sDYNAMIC_EXECUTION=0` so it needs no `eval`.

## portability findings

what B1 found about a native C++ build on Windows and Linux, for the ecosystem's conventions:

- **native on Windows:** Visual Studio 2022 Build Tools 17.6 (MSVC 19.36) builds the engine and its tests under `/W4 /WX`, through the Visual Studio generator, which needs no developer prompt. MinGW-w64 GCC 12 (MSYS2) builds the engine under `-Werror` as well, with Ninja.
- **WebAssembly on Windows:** `emsdk` 7.0.1 installs and runs natively. its `emsdk_env` sets `EMSDK`, which the `wasm` preset's toolchain path reads, and Ninja must be on `PATH`, since Visual Studio's generator can't drive Emscripten. `--emit-tsd` works there too, running Emscripten's bundled Node.
- **the drive letter:** CTest and Vitest pass whether the command starts from `c:\` or `C:\`. Vitest needs the alias in `web/vite.config.ts` for it, the fix the ecosystem's conventions on portability describe.
- **lint:** clang-format and clang-tidy from PyPI, run through `uv`, give Windows and Linux the same versions without installing LLVM. clang-tidy on Windows reads MSVC's headers unless the compile database names another compiler.
- **clang-tidy on Windows:** it runs only where a compile database is, so the Windows session checked the format alone, and CI's first run was the first to run clang-tidy. it found `#pragma once` in both headers (`portability-avoid-pragma-once`), now include guards. to run clang-tidy on Windows before pushing, pass `--build` a Ninja build, such as MinGW-w64 GCC's with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`.
- **clang-tidy's header filter on Windows:** with a MinGW build's compile database, a header's path mixes slashes, as in `cpp/include\pathfinding/grid.hpp`. so `.clang-tidy`'s `HeaderFilterRegex`, which matched `/` only, skipped every engine header on Windows, and only CI checked them. since M7 it matches either slash, and M7's Windows run found that way what CI would have rejected.
- **Linux:** CI covers it, on `ubuntu-26.04`: GCC 15.2 and Clang 21.1, Clang with the sanitizers, and the WebAssembly build with `emsdk` 7.0.1, which needed nothing Linux-specific.
