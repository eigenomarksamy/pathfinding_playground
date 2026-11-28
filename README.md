# pathfinding playground

an interactive laboratory for graph search: build a map, pick a solver, watch its decisions step by step, and compare how algorithms find a route. the search runs in a C++20 engine, compiled to WebAssembly for a React and TypeScript app in the browser, and natively for tests and benchmarks. there is no server.

its bootstrap is done: the builds, the checks and the deploy are in place. the engine has its grid and its test maps, and the app still shows the bootstrap's probe, a pattern of moving bands, until the solvers and the editor arrive. it runs at [paths.eigenomar.com](https://paths.eigenomar.com). the plan is [docs/plan.md](docs/plan.md), and the design [docs/architecture.md](docs/architecture.md).

## what to install

| | Windows | Linux |
| --- | --- | --- |
| C++ compiler, C++20 | Visual Studio 2022, or its Build Tools, with "Desktop development with C++" | GCC 12 or later, or Clang 16 or later |
| CMake 3.25 or later | [cmake.org](https://cmake.org/download/) or `winget install Kitware.CMake` | your package manager, such as `sudo apt install cmake` |
| Ninja | `winget install Ninja-build.Ninja` | `sudo apt install ninja-build` |
| Emscripten 7.0.1, from `emsdk` | see below | see below |
| Node 24 | [nodejs.org](https://nodejs.org/) or `winget install OpenJS.NodeJS.LTS` | [nodejs.org](https://nodejs.org/) or your package manager |
| `uv`, for the lint script | `winget install astral-sh.uv` | [docs.astral.sh/uv](https://docs.astral.sh/uv/getting-started/installation/) |

`emsdk` installs Emscripten anywhere outside this repository. clone it once:

```sh
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
emsdk install 7.0.1        # ./emsdk on Linux
emsdk activate 7.0.1
```

then, in each new terminal before building the WebAssembly module, load its environment: `emsdk_env.bat` in Command Prompt, `emsdk_env.ps1` in PowerShell, or `source ./emsdk_env.sh` on Linux. `emsdk activate 7.0.1 --permanent` on Windows sets it for every terminal instead.

## build and test

from the repository's root, the native engine and its tests:

```sh
cmake --workflow --preset windows-msvc     # Windows
cmake --workflow --preset linux-gcc        # Linux, or linux-clang, or linux-sanitize
```

each configures, builds and runs the tests into `build/<preset>/`. to run the tests again, `ctest --preset <preset>`. the benchmark is `build/windows-msvc/benchmarks/RelWithDebInfo/pathfinding_benchmark` on Windows and `build/<preset>/benchmarks/pathfinding_benchmark` on Linux.

then the web app, from `web/`, with `emsdk`'s environment loaded:

```sh
npm ci
npm run wasm      # builds the WebAssembly module into src/engine/wasm/
npm run check     # types, format and tests
npm run dev       # the app at http://localhost:5173
npm run build     # the production build in dist/
```

and the C++ lint, from the root: `uv run scripts/lint.py`, or `uv run scripts/lint.py --fix` to format the files first. it runs clang-tidy too when a Ninja build's compile database is there, as `build/linux-clang/` is once that preset is configured.

## deploy

every push to `main` deploys, once CI's checks pass: `.github/workflows/ci.yml` builds the module and the app, and publishes the app to Cloudflare Workers static assets at `paths.eigenomar.com`. it needs two repository secrets, under **Settings > Secrets and variables > Actions**:

| secret | value |
| --- | --- |
| `CLOUDFLARE_API_TOKEN` | an account API token made from the **Edit Cloudflare Workers** template, limited to the account and the domain's zone |
| `CLOUDFLARE_ACCOUNT_ID` | the account's ID, which `npx wrangler whoami` shows |

the domain's zone must be on the same Cloudflare account. the first deploy creates the subdomain's DNS record and certificate (`routes` in `wrangler.jsonc`).
