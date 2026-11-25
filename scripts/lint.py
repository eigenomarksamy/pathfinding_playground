# /// script
# requires-python = ">=3.11"
# dependencies = [
#   "clang-format==23.1.5",
#   "clang-tidy==23.1.4",
# ]
# ///
"""Checks the C++ sources' format with clang-format, and the engine's code with clang-tidy.

    uv run scripts/lint.py                  # check the format, and run clang-tidy when a build has a compile database
    uv run scripts/lint.py --fix            # rewrite the files in clang-format's style first
    uv run scripts/lint.py --build build/linux-clang   # the build whose compile_commands.json clang-tidy reads
    uv run scripts/lint.py --require-tidy   # fail rather than skip when there is no compile database (CI)

clang-format and clang-tidy come pinned from PyPI, so every platform runs the same versions.
clang-tidy needs a compile database, which the Ninja presets write: on Linux, configure any
`linux-*` preset first. the Visual Studio generator of `windows-msvc` writes none, so on
Windows only the format is checked unless `--build` names a Ninja build.
"""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FORMAT_DIRS = ("cpp", "benchmarks")
FORMAT_SUFFIXES = {".cpp", ".hpp"}
# the engine and the benchmark. the tests and the bindings are left out (see .clang-tidy).
TIDY_DIRS = ("cpp/src", "benchmarks")


def tool(name: str) -> str:
    found = shutil.which(name)
    if found is None:
        sys.exit(f"lint: {name} isn't on PATH. run this script with `uv run`, which installs it.")
    return found


def sources(dirs: tuple[str, ...]) -> list[Path]:
    files = [
        path
        for directory in dirs
        for path in sorted((ROOT / directory).rglob("*"))
        if path.suffix in FORMAT_SUFFIXES and path.is_file()
    ]
    return files


def check_format(fix: bool) -> bool:
    clang_format = tool("clang-format")
    files = [str(path) for path in sources(FORMAT_DIRS)]
    if fix:
        subprocess.run([clang_format, "-i", *files], check=True)
    result = subprocess.run([clang_format, "--dry-run", "--Werror", *files])
    print(f"clang-format: {len(files)} files, {'clean' if result.returncode == 0 else 'not formatted'}")
    return result.returncode == 0


def run_tidy(build: Path, require: bool) -> bool:
    database = build / "compile_commands.json"
    if not database.is_file():
        message = f"clang-tidy: no {database.relative_to(ROOT) if database.is_relative_to(ROOT) else database}"
        if require:
            print(f"{message}. configure a linux-* preset first.")
            return False
        print(f"{message}, so clang-tidy is skipped. CI runs it on Linux.")
        return True
    clang_tidy = tool("clang-tidy")
    compiled = {Path(entry["file"]).resolve() for entry in json.loads(database.read_text("utf-8"))}
    files = [path for path in sources(TIDY_DIRS) if path.suffix == ".cpp" and path.resolve() in compiled]
    if not files:
        print(f"clang-tidy: {database} compiles none of the engine's sources")
        return False
    result = subprocess.run([clang_tidy, "--quiet", "-p", str(build), *map(str, files)])
    print(f"clang-tidy: {len(files)} files, {'clean' if result.returncode == 0 else 'findings'}")
    return result.returncode == 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--fix", action="store_true", help="format the files in place first")
    parser.add_argument("--build", type=Path, default=ROOT / "build" / "linux-clang",
                        help="the build directory holding compile_commands.json (default: build/linux-clang)")
    parser.add_argument("--require-tidy", action="store_true",
                        help="fail when there is no compile database, rather than skip clang-tidy")
    args = parser.parse_args()

    build = args.build if args.build.is_absolute() else (Path.cwd() / args.build)
    format_ok = check_format(args.fix)
    tidy_ok = run_tidy(build.resolve(), args.require_tidy)
    return 0 if format_ok and tidy_ok else 1


if __name__ == "__main__":
    sys.exit(main())
