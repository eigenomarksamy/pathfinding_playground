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

- **the engine** (`cpp/`) is a static library, `pathfinding_engine`, with no rendering, browser or framework code. both builds compile the same sources. until M1, it holds only the bootstrap's probe, `pathfinding::probe`: a pattern of diagonal bands that moves one cell per step, which tests the builds, the bounds checks and the transfer of a frame.
- **the boundary** (`cpp/bindings/engine_bindings.cpp`) is Embind, and stays small and batched: a call returns a whole frame as one `Uint8Array`, copied into memory JavaScript owns, never one call per cell. the engine returns status codes, not exceptions, so the module is built without C++ exception support. `--emit-tsd` generates `pathfinding.d.ts`, the module's TypeScript declarations, at build time.
- **the adapter** (`web/src/engine/engine.ts`) is the only code that imports the module. it loads it, checks the app's arguments, and turns the engine's statuses and any failure into an `EngineError` with a code: `load_failed`, `invalid_input`, `invalid_size` or `unexpected`.
- **the app** steps the simulation on a fixed clock, apart from the display's frame rate, and caps the steps per frame, so a hidden tab can't flood the engine. with reduced motion asked for, it starts paused.

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

- **native:** GoogleTest under CTest, on MSVC locally, and on GCC, Clang, and Clang with the sanitizers in CI.
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
- **clang-tidy on Windows:** it runs only where a compile database is, so the Windows session checked the format alone, and CI's first run was the first to run clang-tidy. it found `#pragma once` in both headers (`portability-avoid-pragma-once`), now include guards. to run clang-tidy on Windows before pushing, pass `--build` a Ninja build.
- **Linux:** CI covers it, on `ubuntu-26.04`: GCC 15.2 and Clang 21.1, Clang with the sanitizers, and the WebAssembly build with `emsdk` 7.0.1, which needed nothing Linux-specific.
