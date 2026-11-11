#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# check_examples_registry.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

"""
Validate pynest/examples/examples.yml against the files in pynest/examples/.

examples.yml is the single source of truth for PyNEST example metadata.  This
script only reads it; fixing what it reports is a manual edit.

Checks:

  1. Registry sync   - every .py file on disk is referenced by examples.yml in
                       some role, and every registered entry-point path exists
                       on disk.
  2. Required fields - every entry has a name, a path and, unless it is skipped
                       entirely, a non-empty models list; skip is a known value
                       and is always explained in notes.
  3. Auxiliary paths - every helper_scripts, data_files and post_script path
                       exists on disk.

Everything the checks need is in the working tree, so there is no git or network
access and no release tag to pass:

    python .github/scripts/check_examples_registry.py

Exits 0 when examples.yml and pynest/examples/ agree, 1 otherwise.
"""

import argparse
import fnmatch
import sys
from collections.abc import Callable, Iterator
from pathlib import Path
from typing import Final, NotRequired, TypedDict, cast

import yaml


class ExampleEntry(TypedDict):
    """One entry of examples.yml; the fields are documented in that file.

    ``name`` and ``path`` are required: _load_entries() rejects any entry without them.
    """

    name: str
    path: str
    models: NotRequired[list[str]]
    skip: NotRequired[str]
    notes: NotRequired[str]
    helper_scripts: NotRequired[list[str]]
    data_files: NotRequired[list[str]]
    post_script: NotRequired[str]


# Repository root is two levels up from this script (.github/scripts/).
DEFAULT_EXAMPLES_DIR: Final = Path(__file__).resolve().parents[2] / "pynest" / "examples"

# Glob patterns for files and directories that are not examples.
IGNORE_PATTERNS: Final = (
    "__pycache__",
    ".mypy_cache",
    ".ipynb_checkpoints",
    ".git",
    "example_logs",
    "__init__.py",
    "README.rst",
    "README.txt",
    "CMakeLists.txt",
    "DEPENDENCIES.md",
    "*.svg",
    "*.png",
    "*.dat",
    "*.pkl",
    "*.yml",
    "*.yaml",
)

SKIP_VALUES: Final = frozenset({"all", "notebook"})

SEPARATOR: Final = "=" * 72


def _is_ignored(name: str) -> bool:
    return any(fnmatch.fnmatch(name, pattern) for pattern in IGNORE_PATTERNS)


def _is_example_script(path: Path) -> bool:
    return path.is_file() and path.suffix == ".py" and not _is_ignored(path.name)


class ExamplesRegistry:
    """examples.yml together with the pynest/examples/ tree it describes.

    All paths are relative to the examples directory.  Each check yields
    human-readable problem lines, so callers can run any subset.  A registry
    that cannot be used at all (bad YAML, no examples key) raises instead.
    """

    YAML_NAME: Final = "examples.yml"

    def __init__(self, examples_dir: Path) -> None:
        self.examples_dir = examples_dir
        self.yaml_file = examples_dir / self.YAML_NAME
        self.entries = self._load_entries()

    def _load_entries(self) -> list[ExampleEntry]:
        """Parse examples.yml and check that it is a usable registry.

        Raises:
            FileNotFoundError: examples.yml does not exist.
            yaml.YAMLError: examples.yml is not valid YAML.
            ValueError: examples.yml is valid YAML but not a valid registry.
        """
        with self.yaml_file.open(encoding="utf-8") as stream:
            data = yaml.safe_load(stream)

        if not isinstance(data, dict) or "examples" not in data:
            raise ValueError(f"{self.yaml_file}: root must be a mapping with an 'examples' key")
        entries = data["examples"]
        if not isinstance(entries, list):
            raise ValueError(f"{self.yaml_file}: 'examples' must be a list")

        names_by_path: dict[str, str] = {}
        for index, entry in enumerate(entries):
            if not isinstance(entry, dict) or "name" not in entry:
                raise ValueError(f"{self.yaml_file}: example {index} must be a mapping with a 'name' field")
            name = entry["name"]
            if "path" not in entry:
                raise ValueError(f"{self.yaml_file}: example {name!r} has no 'path' field")
            path = entry["path"]
            if path in names_by_path:
                raise ValueError(
                    f"{self.yaml_file}: duplicate path {path!r} in examples {names_by_path[path]!r} and {name!r}"
                )
            names_by_path[path] = name

        return cast(list[ExampleEntry], entries)

    def discover(self) -> Iterator[str]:
        """Yield every .py file in the examples tree and its immediate subdirectories."""
        for item in sorted(self.examples_dir.iterdir()):
            if _is_example_script(item):
                yield item.name
            elif item.is_dir() and not _is_ignored(item.name):
                for sub in sorted(item.iterdir()):
                    if _is_example_script(sub):
                        yield sub.relative_to(self.examples_dir).as_posix()

    def registered_paths(self) -> set[str]:
        """Every path examples.yml references as an entry point, helper or post script."""
        paths: set[str] = set()
        for entry in self.entries:
            paths.add(entry["path"])
            paths.update(entry.get("helper_scripts") or [])
            if post_script := entry.get("post_script"):
                paths.add(post_script)
        return paths

    def check_sync(self) -> Iterator[str]:
        """Check 1: every .py file is registered and every entry-point path exists."""
        registered = self.registered_paths()
        for path in self.discover():
            if path not in registered:
                yield f"{path}: not registered in examples.yml"
        for entry in self.entries:
            if not (self.examples_dir / entry["path"]).is_file():
                yield f"{entry['name']}: path not found: {entry['path']}"

    def check_required_fields(self) -> Iterator[str]:
        """Check 2: every entry carries usable metadata."""
        for entry in self.entries:
            name = entry["name"]
            skip = entry.get("skip")

            if not entry["path"]:
                yield f"{name}: field 'path' is empty"
            if skip is not None:
                if skip not in SKIP_VALUES:
                    yield f"{name}: skip={skip!r} is not one of {sorted(SKIP_VALUES)}"
                if not entry.get("notes"):
                    yield f"{name}: skip is set but 'notes' does not say why"
            if skip != "all" and not entry.get("models"):
                yield f"{name}: field 'models' is missing or empty"

    def check_auxiliary_paths(self) -> Iterator[str]:
        """Check 3: helper_scripts, data_files and post_script point at files that exist."""
        for entry in self.entries:
            post_script = entry.get("post_script")
            fields = {
                "helper_scripts": entry.get("helper_scripts") or [],
                "data_files": entry.get("data_files") or [],
                "post_script": [post_script] if post_script else [],
            }
            for field, paths in fields.items():
                for path in paths:
                    if not (self.examples_dir / path).exists():
                        yield f"{entry['name']}: {field} path not found: {path}"


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Validate examples.yml against the pynest/examples/ directory")
    parser.add_argument(
        "--examples-dir",
        type=Path,
        default=DEFAULT_EXAMPLES_DIR,
        help="path to the pynest/examples/ directory (default: the one in this repository)",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    registry = ExamplesRegistry(args.examples_dir)
    print(f"Examples directory  : {registry.examples_dir}")
    print(f"Registered examples : {len(registry.entries)}")

    checks: list[tuple[str, Callable[[], Iterator[str]]]] = [
        ("Check 1: registry sync", registry.check_sync),
        ("Check 2: required fields", registry.check_required_fields),
        ("Check 3: helper_scripts, data_files and post_script paths", registry.check_auxiliary_paths),
    ]

    failures: list[str] = []
    for title, check in checks:
        print(f"\n{SEPARATOR}\n{title}\n{SEPARATOR}")
        problems = list(check())
        print("\n".join(f"  - {problem}" for problem in problems) if problems else "OK")
        failures.extend(problems)

    if not failures:
        print("\nOK: examples.yml is in sync with pynest/examples/")
        return 0

    print(f"\nFAILED: {len(failures)} issue(s) found.")
    print("Update pynest/examples/examples.yml (field documentation is at the top of the file) and run this again.")
    return 1


if __name__ == "__main__":
    # Only the command line turns a broken registry into an exit code; the
    # functions above raise so that callers can handle failure differently.
    try:
        sys.exit(main())
    except (OSError, ValueError, yaml.YAMLError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        sys.exit(1)
