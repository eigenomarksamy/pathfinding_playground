@../meta-eigenomar/standards/claude-ecosystem.md

# pathfinding playground: Claude instructions

pathfinding playground is an interactive laboratory for graph search: users build a map, pick a solver, watch its decisions step by step, and compare how algorithms find a route. the search runs in a C++20 engine, compiled to WebAssembly for a React and TypeScript app, and natively for tests and benchmarks. it is one of three C++ labs, with `signal_lab` and `gravity_engine`. nothing is built yet. it is planned for `paths.eigenomar.com` and isn't deployed.

## commands

there is no code yet, so there is nothing to build or test. B1, the bootstrap, adds the first code: the CMake presets for native builds on Windows and Linux and for WebAssembly through `emsdk`, and the web app's scripts. when it lands, list its setup, build, test and run commands here, each working on Windows and Linux, along with what to run before pushing.

## plan

[docs/plan.md](docs/plan.md) is the plan. ticket ids are a lane prefix and a number (B1, M1, L1) and are never reused or renumbered. each ticket has a stable anchor, its id in lowercase, above its heading. when work ships, update the ticket's status in the index, add a result with the commit to the ticket, and add a line to the log. this repository is public, so confidential work never goes in the plan or anywhere else here.

## design

every change keeps to the outcomes of shared decision SD3 that the plan's [shared decisions](docs/plan.md#shared-decisions) list: the engine in C++20, separate from rendering and browser code; one engine source for the native and WebAssembly builds; a narrow, batched boundary to JavaScript; bounded inputs; and building, running and testing on Windows and Linux. a change to one of them is made in SD3 first. the MVP's algorithm and data-model constraints are in the plan, under [MVP](docs/plan.md#mvp). B1 settles the layout: record it here or in `docs/architecture.md`, and point `project.yaml` at that document in the same change.

## delivery

there is no CI, no release process and no deployment yet. B1 adds CI on GitHub Actions and a deploy to Cloudflare from it on every merge to `main`, and records both here and in `project.yaml` in the same change.
