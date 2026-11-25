# pathfinding playground plan

an interactive laboratory for graph search. users create maps, select a solver, observe its decisions, and compare how quickly and efficiently algorithms find a route. the search runs in C++20, compiled to WebAssembly for the browser and natively for tests and benchmarks, with no server. it is planned for `paths.eigenomar.com`.

nothing is built yet. this plan holds the bootstrap (B1), the MVP (M1–M5) phase 2 (L1–L6), the adoption of the ecosystem's standards (DOC1, SCM1), and the shared decisions they rely on. the playground is the first of three C++ labs, with `signal_lab` and `gravity_engine`, each in its own repository: they reuse its bootstrap approach, not its code. work across the three, such as their release order or a library they might later share, is coordinated in the ecosystem, not planned here.

## how this plan works

- **tickets.** every ticket has an id, a lane prefix and a number, never reused or renumbered. a split ticket keeps its number and adds a letter (M2a, M2b). each ticket has a stable anchor, its id in lowercase (`#m1`), above its heading, which doesn't change when the heading does.
- **lanes.**

  | prefix | lane |
  | --- | --- |
  | B | the bootstrap: native, WebAssembly and web builds, CI and the deploy |
  | M | the MVP, released at `paths.eigenomar.com` |
  | L | phase 2: later work, after the MVP's release |
  | DOC | the lab's documents: vision, architecture, design, testing strategy, and verification and validation |
  | SCM | source control and delivery: CI, branching, protection, releases and versioning |

- **each ticket** says what to do, what it needs, and when it is done. its "done when" is its acceptance criteria. a need is a hard need: the ticket starts once it is done.
- **size** is the index's column, on the ecosystem's scale, the same value as the ticket's issue's `Size` in the ecosystem's GitHub Project: xxs takes minutes, xs under an hour, s an hour or two, m about a session, l two or three, xl four to six and xxl more. tbd means an open decision or work not yet planned decides it. a change to one is made to the other in the same session.
- **issues.** every open ticket gets one GitHub issue in this repository. the issue's title starts with the ticket's id, and its body links the ticket's anchor here without copying it. the ticket holds the scope and history, and the issue holds status, priority and blockers.
- **shared decisions** bind this lab and other repositories. this plan states each outcome it relies on in one line and cites the decision by its id. a change to one is made in the decision first.
- **adding work.** new work goes here as a ticket in a lane. this repository is public, so confidential work never goes in this file. the ecosystem's shared rules, which `CLAUDE.md` imports, say where it goes.
- **when work ships,** update the ticket's status in the index, add a result with the commit to the ticket, and add a line to the [log](#log).

## shared decisions

the three C++ labs share their constraints, stack, architecture and definition of done as shared decision SD3, which makes no shared package mandatory. the outcomes this lab relies on:

- **its own app:** it works on its own at its own URL, with no account, login, payment or database, and needs no backend, external API, uploaded data or real-time third-party service.
- **usable at once:** the default experience starts with sample maps and sensible parameters.
- **controls:** play, pause, reset, and step wherever stepping is meaningful.
- **seeds:** anything random takes a reproducible seed, and a preset gives the same outcome for the same configuration.
- **honest models:** educational approximations are clearly told apart from validated models.
- **access:** controls work by keyboard and the layout is responsive; the visualization may favor desktop, with a working mobile fallback.
- **motion:** reduced-motion preferences are honored for nonessential animation, and nothing flashes excessively or animates without a way to pause it.
- **privacy:** no analytics or tracking by default; only optional preferences are kept, locally.
- **identity:** the three labs look consistent, without a shared frontend package before one is needed.
- **footer:** the ecosystem's button to `https://eigenomar.com`, labelled `eigenomar.com`, a plain link in the app's own button style, opening in a new tab here, since leaving would lose an edited map.
- **C++20:** the engine is C++20; C++23 features wait until the pinned compiler and Emscripten are shown to support them.
- **separation:** the engine is separate from rendering, browser events and framework code; algorithm state stays in C++, and TypeScript does the controls, state coordination and drawing.
- **the boundary:** a small, documented interface to JavaScript, through Embind at first, transferring batched state rather than one call per explored node.
- **two builds:** one engine source compiles to a native build and a WebAssembly build.
- **C++ habits:** RAII, clear ownership, `const` correctness, explicit types, `std::span` and standard containers, without premature templates or allocations in frame-critical paths.
- **steps:** simulation steps are decoupled from the visual frame rate, fixed-step where that applies.
- **limits:** inputs are bounded and invalid parameters handled, so no scenario can freeze the tab.
- **threads:** single-threaded at first, with a standard Web Worker for long computation if needed; C++ pthreads aren't a baseline.
- **metrics:** performance is measurable, here as execution time and explored nodes.
- **portability:** it builds, runs and tests on Windows and Linux.
- **the stack:** CMake, Emscripten and GoogleTest with CTest for the engine; React, TypeScript and Vite with Canvas 2D for the app, WebGL2 only if profiling justifies it, tested with Vitest and Playwright; clang-format, clang-tidy, compiler warnings and sanitizers in native CI; GitHub Actions for CI and deploys; Cloudflare Workers static assets on its own subdomain for hosting.
- **dependencies:** few and pinned, through one dependency-management method chosen when implementation starts, such as CMake's `FetchContent` at pinned revisions.
- **the architecture:** a React and TypeScript UI, over a typed Wasm adapter (input validation and conversion, batched snapshots or events, the module's lifecycle and errors), over a WebAssembly module of the C++20 engine, which a native test and benchmark executable also builds.
- **the layout:** the suggested one, with `cpp/` (include, src, bindings, tests), `web/`, `benchmarks/`, `docs/`, `CMakeLists.txt`, `CMakePresets.json`, `wrangler.jsonc` and `.github/workflows/` beside `CLAUDE.md`, `project.yaml`, `README.md`, `LICENSE` and `.gitattributes`.
- **no shared library yet:** common code is extracted only once at least two labs need it.
- **done:** each public release meets SD3's definition of done, which [M5](#m5)'s done-when cites.

every app under eigenomar.com shows the same tab icon, eigenomar.com's lambda with its two colors swapped, a light-blue tile and a near-black lambda: shared decision SD5. [B1](#b1)'s shell takes it, so the lab is never deployed without it.

## index

| ticket | title | size | needs | status |
| --- | --- | --- | --- | --- |
| [B1](#b1) | bootstrap: native, WebAssembly and web, built and deployed from GitHub Actions | xl | none | in progress |
| [M1](#m1) | core grid and test fixtures | m | B1 | todo |
| [M2](#m2) | BFS, Dijkstra and A*, with heuristics and metrics | m | M1 | todo |
| [M3](#m3) | compact event trace and Wasm interface | m | M2 | todo |
| [M4](#m4) | grid editor, animation, controls and example maps | l | M3 | todo |
| [M5](#m5) | release: accessibility, tests, deployment and README | m | M4 | todo |
| [L1](#l1) | bidirectional search and weighted A* | m | M5 | todo |
| [L2](#l2) | ant colony optimization and genetic algorithms | l | M5 | todo |
| [L3](#l3) | maze generation, and map import and export | m | M5 | todo |
| [L4](#l4) | comparison mode | m | M5 | todo |
| [L5](#l5) | continuous motion planning with RRT and RRT* | l | M5 | todo |
| [L6](#l6) | text execution trace and grid descriptions | m | M5 | todo |
| [DOC1](#doc1) | the lab's documents | tbd | `meta-eigenomar` ST1 | todo |
| [SCM1](#scm1) | the ecosystem's SCM strategy | tbd | `meta-eigenomar` ST2 | todo |

## bootstrap

stage 0: the builds, checks and deploy every later ticket uses, set up once. `signal_lab` and `gravity_engine` reuse its approach, not its code, so it needn't become a common dependency, and the approach is updated as issues become clear.

<a id="b1"></a>

### B1 · bootstrap: native, WebAssembly and web, built and deployed from GitHub Actions

- **do:** the checklist below. this ticket adds the repository's first code, its CI and its deploy.
  - **portability:** `CMakePresets.json` has native presets for Windows (MSVC or clang-cl) and Linux (GCC or Clang), and a WebAssembly preset through Emscripten. Emscripten comes from `emsdk`, which runs on Windows natively. no build step exists only as a `.sh` or `.ps1` script, and nothing needs WSL. CI runs the native tests, with sanitizers, and the WebAssembly build on `ubuntu-latest`.
  - **delivery:** Cloudflare Workers serve the static assets on `paths.eigenomar.com`. GitHub Actions builds the WebAssembly module and deploys the result, since Cloudflare's own build doesn't run Emscripten, on every merge to `main`.
- **needs:** nothing.
- **done when:**
  - [x] a minimal C++20 library builds with CMake, with native tests and one benchmark target.
  - [x] that library compiles with Emscripten, and TypeScript calls a simple function it exports.
  - [x] a stable typed adapter handles Wasm initialization, errors and data transfer.
  - [x] linting and formatting, native sanitizers and CI checks are configured.
  - [x] it builds natively on Windows and in CI on Linux, with Emscripten from `emsdk` on both, and the native tests pass on Windows started from `c:\` as well as from `C:\`.
  - [x] a minimal Vite and React shell has a canvas, basic controls and an about/source panel.
  - [x] the shell's tab shows the ecosystem's icon (shared decision SD5), served from the app's own assets, in a light and a dark browser theme.
  - [ ] a production static deployment on Cloudflare, built and deployed from GitHub Actions, answers at `paths.eigenomar.com`.
  - [x] the README lists what to install on each platform: the compiler, CMake, `emsdk` and Node. `CLAUDE.md`'s commands section lists the setup, build, test and run commands, each working on Windows and Linux.
  - [x] in the same change as the deploy, `project.yaml`'s `delivery` says `ci: github-actions`, `deployment: cloudflare` and `deploys_on: merge`, its `links.deployment` is `https://paths.eigenomar.com`, and `CLAUDE.md`'s delivery section says what CI runs and that every merge to `main` deploys. the ecosystem's validator passes.
  - [ ] the result records what worked for the native and WebAssembly builds on each platform, for the ecosystem's conventions on native builds.
- **progress, 25 november 2026:** the first session built everything but what needs CI's first run and the deploy: the engine with a probe in place of the grid, its GoogleTest tests and benchmark, the five presets, the Embind module with generated TypeScript declarations, the typed adapter, the shell with the icon, the lint script, `ci.yml` with its deploy job, `wrangler.jsonc`, and [docs/architecture.md](architecture.md), which records the layout and the portability findings so far. on Windows the native workflow (12 tests), the WebAssembly build, `npm run check` (13 tests) and the lint pass, the tests from `c:\` and `C:\` alike, and the built app runs under its content security policy in Chromium, in a light and a dark theme. left: the repository's Cloudflare secrets, CI's first run on Linux with the sanitizers and clang-tidy, the first deploy answering at `paths.eigenomar.com`, and the result.
- **progress, 25 november 2026, later:** CI's first run, at 151b6a3, passed every job but the lint, where clang-tidy, which runs only in CI, rejected `#pragma once` (`portability-avoid-pragma-once`). 8229791 moved both headers to include guards, and its run passed the native jobs on GCC 15.2 and Clang 21.1 with the sanitizers (12 tests each), the lint, and the WebAssembly build and app checks with Emscripten 7.0.1, on `ubuntu-26.04`. its deploy failed only because the repository has no Cloudflare secrets yet. left: the secrets, the deploy answering at `paths.eigenomar.com`, and the result.

## MVP

stage 1: one polished, bounded release at `paths.eigenomar.com`. the core's correctness tests come before any time spent on animation, so M1–M3 come before M4. the MVP's acceptance criteria are spread over M1–M5's done-whens, each in the ticket that delivers it, and M5 releases the MVP once they all hold.

**scope:**

- editable rectangular grid with **start**, **goal**, **walls**, and optional weighted terrain.
- algorithms: **BFS**, **Dijkstra**, and **A\***.
- deterministic step-by-step visualization of the **frontier**, **visited nodes**, and **final path**.
- controls: play, pause, single step, restart, speed, grid size, and preset maps.
- side-by-side metrics: path cost, path length, expanded nodes, runtime, and result state (`found`, `unreachable`, `cancelled`).
- choose **4-way** or **8-way** movement and display the exact movement/corner-cutting policy.
- provide a short explanation of each algorithm and when it is appropriate.

**algorithm and data-model constraints:**

- use the same grid graph and cost definition for every algorithm so results are comparable.
- BFS is only a shortest-path baseline when all traversable edges have equal cost. disable it for weighted/diagonal-cost comparisons or explicitly explain its limitation.
- Dijkstra must handle nonnegative edge costs. reject negative weights.
- A* must use a **consistent/admissible** heuristic for the configured movement model. Manhattan for 4-way unit steps; octile for 8-way diagonal costs when appropriate.
- define a no-corner-cutting rule for diagonal moves, and apply it consistently.
- distinguish algorithm compute time from animation time; do not use rendered frame rate as a search benchmark.
- store an inspectable search trace or bounded batches of events, rather than binding the renderer to the internals of the search algorithm.
- handle start equals goal, no path, blocked cells, tiny grids, and large grids safely.

**proposed C++ modules:**

```text
grid / coordinate / movement policy
search result + event types
breadth-first search
dijkstra search
A* search
heuristics
metrics + reproducible map generation
```

<a id="m1"></a>

### M1 · core grid and test fixtures

- **do:** the grid, its coordinates and its movement policy: a rectangular grid with start, goal, walls and optional weighted terrain, 4-way or 8-way movement, and the no-corner-cutting rule for diagonal moves, in one grid graph and cost definition that every algorithm uses. reproducible map generation from a seed, and the canonical test maps the algorithms are tested on.
- **needs:** B1.
- **done when:**
  - native unit tests cover 4-way and 8-way neighbors and their costs, weighted terrain included, and the no-corner-cutting rule.
  - a map generated twice from the same seed is identical, and a test checks it.
  - canonical test maps exist for an open grid, walls, weighted terrain, start equal to goal, no path, blocked cells, a tiny grid and a large grid, and each loads without error.

<a id="m2"></a>

### M2 · BFS, Dijkstra and A*, with heuristics and metrics

- **do:** BFS, Dijkstra and A* on M1's grid, A*'s heuristics, and each run's result and metrics: path cost, path length, expanded nodes, runtime and result state (`found`, `unreachable`, `cancelled`). runtime is the algorithm's compute time, never animation or frame time.
- **needs:** M1.
- **done when:**
  - BFS, Dijkstra and A* return the expected results on the canonical test maps, start equal to goal, no path, blocked cells, and the tiny and large grids included.
  - Dijkstra and A* agree on the minimum path cost wherever the heuristic is valid, with Manhattan for 4-way unit steps and octile for 8-way diagonal costs.
  - a negative weight is rejected with an error, never searched.
  - every run reports its path cost, path length, expanded nodes, compute time and result state, and a cancelled run reports `cancelled`.

<a id="m3"></a>

### M3 · compact event trace and Wasm interface

- **do:** each search records an inspectable trace of its events, or bounded batches of them, so the renderer reads the trace and never the search's internals. the engine's Embind interface hands the app the run, its metrics and its trace in batches, through B1's typed adapter.
- **needs:** M2.
- **done when:**
  - the same seed, input and algorithm yield an identical search trace, and a test checks it.
  - the trace crosses the Wasm boundary in bounded batches, never one call per explored node.
  - the native and WebAssembly builds pass the same correctness fixtures.
  - invalid input from the app comes back through the adapter as a typed error, not a crash.

<a id="m4"></a>

### M4 · grid editor, animation, controls and example maps

- **do:** the MVP's scope in the app, on B1's shell: the grid editor, the algorithm and movement choice with the exact movement and corner-cutting policy shown, the step-by-step frontier, visited nodes and final path from M3's trace, the controls, the preset maps, the metrics side by side, and a short explanation of each algorithm and when it is appropriate.
- **needs:** M3.
- **done when:**
  - changing the algorithm updates the metrics and the visual output meaningfully.
  - BFS is disabled for weighted or diagonal-cost comparisons, or its limitation is explained where it is chosen.
  - the runtime shown is M2's compute time, apart from animation time.
  - interactions stay responsive on a documented default grid, around 40 × 30 cells, and larger grids are bounded and profiled.
  - each preset map loads, and play, pause, single step, restart and speed work on it.
  - the footer's `eigenomar.com` button opens `https://eigenomar.com` in a new tab, so an edited map survives.

<a id="m5"></a>

### M5 · release: accessibility, tests, deployment and README

- **do:** an accessibility pass, the app's tests, the MVP's deployment at `paths.eigenomar.com` through B1's pipeline, and the README.
- **needs:** M4.
- **done when:**
  - shared decision SD3's definition of done for each public release holds for everything this repository delivers. its items that other repositories deliver once the demo is live, the site's software entry among them, follow this ticket.
  - the deployed demo, the public README and the source link work without a backend.
  - every control works by keyboard, the layout has a working mobile fallback, and a reduced-motion preference stops nonessential animation.
  - Vitest covers the app's logic and a Playwright smoke test the deployed build, both in CI.
  - M1–M4's done-whens hold on the deployed build.

## phase 2

not MVP. it starts once the MVP is released ([M5](#m5)), and keeps the advanced algorithms out of the MVP's scope. each ticket uses the MVP's grid, trace and metrics where it can.

<a id="l1"></a>

### L1 · bidirectional search and weighted A*

- **do:** bidirectional search and **weighted A*** with clear optimality tradeoffs.
- **needs:** M5.
- **done when:** both run on the canonical test maps, tests compare their path costs with Dijkstra's, and the app shows each one's cost beside the minimum and explains its optimality tradeoff.

<a id="l2"></a>

### L2 · ant colony optimization and genetic algorithms

- **do:** metaheuristic solvers such as **ant colony optimization** and **genetic algorithms**, compared with the exact solvers on the same maps.
- **needs:** M5.
- **done when:** both run on the same maps as BFS, Dijkstra and A*, the same seed and map give the same result, and the app shows their results beside the exact solvers' and says they are approximations that need not find the minimum.

<a id="l3"></a>

### L3 · maze generation, and map import and export

- **do:** maze generation and imported/exported map definitions.
- **needs:** M5.
- **done when:** the same seed generates the same maze, an exported map imports back unchanged, and an invalid or oversized import is rejected with a message.

<a id="l4"></a>

### L4 · comparison mode

- **do:** comparison mode with synchronized runs and a summary chart.
- **needs:** M5.
- **done when:** two or more algorithms run in step on one map under one set of controls, and a summary chart compares their metrics.

<a id="l5"></a>

### L5 · continuous motion planning with RRT and RRT*

- **do:** continuous 2D **motion planning** with **RRT / RRT*** as a separate scene type, not a forced extension of grid search.
- **needs:** M5.
- **done when:** a continuous scene type, separate from the grid, plans with RRT and RRT* from a seed, the same seed gives the same plan, and tests check that a found path avoids every obstacle.

<a id="l6"></a>

### L6 · text execution trace and grid descriptions

- **do:** accessibility enhancements: text-based execution trace and alternative grid descriptions.
- **needs:** M5.
- **done when:** a run's trace can be read step by step as text, and the grid has a text description of its start, goal, walls and terrain, both reachable by keyboard and by a screen reader.

## ecosystem standards

the ecosystem's contract, in `meta-eigenomar`, is adding two standards that every repository adopts: the documents each one keeps, and how each one manages its source and delivers it. DOC1 and SCM1 adopt them here, each once its standard is designed. until then, their issues stay in the backlog. neither needs B1, and B1 needs neither.

<a id="doc1"></a>

### DOC1 · the lab's documents

- **do:** write this lab's product vision, architecture, design, testing strategy, and verification and validation (V&V) documents, in the shape the ecosystem's document standard sets. where a file or section already covers one, name it rather than copy it. today the vision is this plan's opening, the architecture and the stack are shared decision SD3's outcomes under [shared decisions](#shared-decisions), the MVP's scope and constraints are under [MVP](#mvp), and [M5](#m5)'s done-when holds the release's checks. `CLAUDE.md` asks B1 to record the layout in `docs/architecture.md` or `CLAUDE.md`, so whichever lands second gives that document its place among the five. written before B1, the documents describe the planned lab, and B1 and the MVP keep them true.
- **needs:** the ecosystem's document standard (`meta-eigenomar` ST1).
- **done when:** each of the five documents exists, or names the file or section that holds it, and follows the standard's outline. `project.yaml` names them under the standard's keys, and the ecosystem's validator passes. nothing in them is confidential, since this repository is public.

<a id="scm1"></a>

### SCM1 · the ecosystem's SCM strategy

- **do:** adopt the ecosystem's SCM strategy through its adoption checklist: CI, branching, protection of `main`, versioning and releases, the health report and dependency updates. [B1](#b1) plans CI on GitHub Actions and a deploy on every merge to `main`. if the strategy is ready before B1 starts, B1 follows it and this ticket checks the rest. otherwise this ticket brings B1's workflows in line.
- **needs:** the ecosystem's SCM strategy (`meta-eigenomar` ST2).
- **done when:** every item on the checklist is applied here, or recorded here as an exception the strategy allows, with its reason. `project.yaml`'s `delivery` and `CLAUDE.md` say what runs, any workflow passes on `main`, and the ecosystem's validator passes.

## log

- 2026-11-25 · B1 · in progress: CI passes on Linux at 8229791, after clang-tidy's first run moved the headers to include guards. the deploy waits for the repository's Cloudflare secrets.
- 2026-11-25 · B1 · in progress: the builds, checks, shell and deploy workflow, verified on Windows. CI's first run on Linux and the first deploy are left, as B1's progress note says.
- 2026-11-20 · B1 · added the ecosystem's tab icon (shared decision SD5) to B1's checklist, at the owner's request, rather than a ticket of its own: the shell B1 builds is the first page the lab deploys. B1's size is unchanged.
- 2026-11-18 · plan · added DOC1 and SCM1, in two new lanes, at the owner's request: this repository's adoption of the ecosystem's document standard and SCM strategy, which are designed in `meta-eigenomar`. each needs its standard, so its issue waits in the backlog.
- 2026-11-14 · plan · created by the ecosystem setup session, from the three C++ labs' shared plan, where this lab's part is now a link here. its stage 0 became B1, its section the MVP's scope and constraints with stage 1's five steps as M1–M5, and its phase 2 L1–L6, in the same order. the MVP's seven acceptance criteria are in M2–M5's done-whens. the shared constraints and the definition of done are shared decision SD3, cited by its id. `project.yaml` validates, with no CI, releases or deployment until B1, `CLAUDE.md` imports the ecosystem's shared rules, and `.gitattributes` holds `* text=auto eol=lf`. nothing else in the repository changed.
