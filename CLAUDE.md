@../meta-eigenomar/standards/claude-ecosystem.md

# pathfinding playground: Claude instructions

pathfinding playground is an interactive laboratory for graph search: users build a map, pick a solver, watch its decisions step by step, and compare how algorithms find a route. the search runs in a C++20 engine, compiled to WebAssembly for a React and TypeScript app, and natively for tests and benchmarks. it is one of three C++ labs, with `signal_lab` and `gravity_engine`. the bootstrap's builds, checks and deploy are in place, and the engine has M7's grid and canonical test maps. the app still draws the bootstrap's probe until M9's interface and M10's editor bring the grid to it. it deploys to `paths.eigenomar.com`.

## commands

the README lists what to install on each platform. every command works on Windows and Linux. the C++ commands run from the root, and the npm ones from `web/`.

- **native build and tests:** `cmake --workflow --preset windows-msvc` on Windows, `cmake --workflow --preset linux-gcc` (or `linux-clang`, `linux-sanitize`) on Linux. it configures, builds and runs CTest into `build/<preset>/`. `ctest --preset <preset>` reruns the tests.
- **WebAssembly module:** `npm run wasm`, with `emsdk`'s environment loaded (`emsdk_env`), so `EMSDK` is set and Ninja is on `PATH`. it installs the module into `web/src/engine/wasm/`, which isn't committed, and the app's type check, tests and build need it.
- **web setup and checks:** `npm ci`, then `npm run check` (TypeScript, Prettier's check and Vitest, whose adapter tests load the real module), and `npm run format` to fix the format.
- **run:** `npm run dev` serves the app at `http://localhost:5173`, and `npm run build` writes the production build to `web/dist/`.
- **C++ lint:** `uv run scripts/lint.py`, or `--fix` to format first. clang-tidy runs when a Ninja build's compile database is there, and always in CI.
- **benchmark:** `pathfinding_benchmark` in the native build's `benchmarks/` folder, apart from any drawing.
- **before pushing:** the native workflow, `npm run wasm`, `npm run check`, `npm run build` and `uv run scripts/lint.py`. on Windows, check the tests from a lowercase `c:` too: in `cmd`, `cd /d` into the clone with the drive spelled `c:`, then run `ctest --preset windows-msvc` and, in `web/`, `npm test`. PowerShell's `Set-Location` turns the drive letter into `C:`, so it can't check this.

## plan

[docs/plan.md](docs/plan.md) is the plan. ticket ids are a lane prefix and a number (B1, M7, L15) and are never reused or renumbered. each ticket has a stable anchor, its id in lowercase, above its heading. when work ships, update the ticket's status in the index, add a result with the commit to the ticket, and add a line to the log. this repository is public, so confidential work never goes in the plan or anywhere else here.

## design

every change keeps to the outcomes of shared decision SD3 that the plan's [shared decisions](docs/plan.md#shared-decisions) list: the engine in C++20, separate from rendering and browser code; one engine source for the native and WebAssembly builds; a narrow, batched boundary to JavaScript; bounded inputs; and building, running and testing on Windows and Linux. a change to one of them is made in SD3 first. the MVP's algorithm and data-model constraints are in the plan, under [MVP](docs/plan.md#mvp). the layout, the builds, the boundary and the checks are in [docs/architecture.md](docs/architecture.md), which `project.yaml` names. a change to them updates it in the same change.

## delivery

`.github/workflows/ci.yml` runs on every push and pull request, on `ubuntu-latest`: the native tests with GCC, Clang, and Clang with AddressSanitizer and UndefinedBehaviorSanitizer, the C++ lint, and the web job, which builds the WebAssembly module with Emscripten 7.0.1, then checks and builds the app. every merge to `main`, and every push to it, deploys: once those jobs pass, its `deploy` job publishes the app to Cloudflare Workers static assets at `paths.eigenomar.com`, with the `CLOUDFLARE_API_TOKEN` and `CLOUDFLARE_ACCOUNT_ID` secrets. `project.yaml` says `deploys_on: merge`, so merging is deploying, and the developer approves it. a change to Emscripten's version changes it in `ci.yml` and the README together. each job ends with a summary of what it did, as the ecosystem's conventions on workflows ask.
