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
import logging
import sys
from collections.abc import Iterator
from pathlib import Path
from typing import Final, NotRequired, TypedDict, cast

import yaml

logger = logging.getLogger(__name__)


class ExampleEntry(TypedDict):
    """One entry of examples.yml, as the fields are documented in that file.

    ``name`` and ``path`` are not NotRequired because ExamplesRegistry rejects
    any entry lacking them before an entry is ever seen as an ExampleEntry.
    """

    name: str
    path: str
    models: NotRequired[list[str]]
    skip: NotRequired[str]
    notes: NotRequired[str]
    helper_scripts: NotRequired[list[str]]
    data_files: NotRequired[list[str]]
    post_script: NotRequired[str]


# The examples directory, relative to the repository root, which is two levels
# up from this script (.github/scripts/).
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


def _should_ignore(name: str) -> bool:
    return any(fnmatch.fnmatch(name, pattern) for pattern in IGNORE_PATTERNS)


def _is_example_script(path: Path) -> bool:
    return path.is_file() and path.suffix == ".py" and not _should_ignore(path.name)


class ExamplesRegistry:
    """examples.yml together with the pynest/examples/ tree it describes.

    Every path handled here is relative to the examples directory, so neither
    the repository root nor any absolute path has to be threaded through.

    The checks are independent and each yields human-readable problem lines, so
    a caller can run any subset and report them together.  Problems that make
    the registry unusable (bad YAML, no examples key) raise instead.
    """

    YAML_NAME = "examples.yml"

    def __init__(self, examples_dir: Path) -> None:
        self.examples_dir = examples_dir
        self.yaml_file = examples_dir / self.YAML_NAME
        self.entries = self._load_entries()

    # -- loading and structural validation ------------------------------------

    def _load_entries(self) -> list[ExampleEntry]:
        """Parse examples.yml and check that it is a usable registry.

        Raises:
            FileNotFoundError: examples.yml does not exist.
            yaml.YAMLError: examples.yml is not valid YAML.
            ValueError: examples.yml is valid YAML but not a valid registry.
        """
        with self.yaml_file.open(encoding="utf-8") as stream:
            data = yaml.safe_load(stream)

        if not isinstance(data, dict):
            raise ValueError(f"{self.yaml_file}: root must be a mapping")
        if "examples" not in data:
            raise ValueError(f"{self.yaml_file}: no 'examples' key")

        entries = data["examples"]
        if not isinstance(entries, list):
            raise ValueError(f"{self.yaml_file}: 'examples' must be a list")

        registered_by_path: dict[str, str] = {}
        for index, entry in enumerate(entries):
            if not isinstance(entry, dict):
                raise ValueError(f"{self.yaml_file}: example {index} must be a mapping")
            if "name" not in entry:
                raise ValueError(f"{self.yaml_file}: example {index} has no 'name' field")
            if "path" not in entry:
                raise ValueError(f"{self.yaml_file}: example {entry['name']!r} has no 'path' field")
            path = entry["path"]
            if path in registered_by_path:
                raise ValueError(
                    f"{self.yaml_file}: duplicate path {path!r} in examples "
                    f"{registered_by_path[path]!r} and {entry['name']!r}"
                )
            registered_by_path[path] = entry["name"]

        return cast(list[ExampleEntry], entries)

    # -- the files on disk ----------------------------------------------------

    def discover(self) -> Iterator[str]:
        """Yield every .py file in the examples tree, entry points and helpers alike.

        Telling entry points from helpers is examples.yml's job; check_sync()
        filters out everything the registry already accounts for in any role.
        """
        for item in sorted(self.examples_dir.iterdir()):
            if _is_example_script(item):
                yield item.name
            elif item.is_dir() and not _should_ignore(item.name):
                for path in sorted(sub for sub in item.iterdir() if _is_example_script(sub)):
                    yield str(path.relative_to(self.examples_dir))

    @property
    def registered_paths(self) -> set[str]:
        """Every path examples.yml references, in any role."""
        paths: set[str] = set()
        for entry in self.entries:
            paths.add(entry["path"])
            paths.update(entry.get("helper_scripts") or [])
            if entry.get("post_script"):
                paths.add(entry["post_script"])
        return paths

    # -- the checks -----------------------------------------------------------

    def check_sync(self) -> tuple[list[str], list[ExampleEntry]]:
        """Check 1: compare the files on disk with the registry.

        Returns:
            (missing, orphaned) - files no entry refers to, and entries whose
            entry-point path no longer exists on disk.
        """
        registered = self.registered_paths
        missing = [path for path in self.discover() if path not in registered]
        orphaned = [entry for entry in self.entries if not (self.examples_dir / entry["path"]).exists()]
        return missing, orphaned

    def check_required_fields(self) -> Iterator[str]:
        """Check 2: every entry carries usable metadata."""
        for entry in self.entries:
            name = entry["name"]
            skip = entry.get("skip")

            if not entry["path"]:
                yield f"{name}: field 'path' is empty"
            if skip is not None and skip not in SKIP_VALUES:
                yield f"{name}: skip={skip!r} is not one of {sorted(SKIP_VALUES)}"
            if skip is not None and not entry.get("notes"):
                yield f"{name}: skip is set but 'notes' does not say why"
            if skip != "all" and not entry.get("models"):
                yield f"{name}: field 'models' is missing or empty"

    def check_auxiliary_paths(self) -> Iterator[str]:
        """Check 3: helper_scripts, data_files and post_script point at files that exist."""
        for entry in self.entries:
            post_script = entry.get("post_script")
            listed: list[tuple[str, list[str]]] = [
                ("helper_scripts", entry.get("helper_scripts") or []),
                ("data_files", entry.get("data_files") or []),
                ("post_script", [post_script] if post_script else []),
            ]
            for field, paths in listed:
                for path in paths:
                    if not (self.examples_dir / path).exists():
                        yield f"{entry['name']}: {field} path not found: {path}"


def _section(title: str) -> None:
    print(f"\n{SEPARATOR}\n{title}\n{SEPARATOR}")


def _report(problems: list[str], all_clear: str) -> None:
    print("\n".join(f"  - {problem}" for problem in problems) if problems else all_clear)


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Validate examples.yml against the pynest/examples/ directory")
    parser.add_argument(
        "--examples-dir",
        type=Path,
        default=DEFAULT_EXAMPLES_DIR,
        help="Path to the pynest/examples/ directory (default: the one in this repository)",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    logging.basicConfig(format="%(levelname)s: %(message)s", level=logging.WARNING)
    args = parse_args(argv)

    registry = ExamplesRegistry(args.examples_dir)
    print(f"Examples directory  : {registry.examples_dir}")
    print(f"Registered examples : {len(registry.entries)}")

    failures: list[str] = []

    _section("Check 1: registry sync")
    missing, orphaned = registry.check_sync()
    if missing:
        print(f"{len(missing)} file(s) on disk are not registered in examples.yml:")
        print("\n".join(f"  - {path}" for path in missing))
        print("\nAdd an entry for each to pynest/examples/examples.yml.")
        print("See the field documentation at the top of that file.")
        failures.extend(f"{path}: not registered in examples.yml" for path in missing)
    if orphaned:
        print(f"{len(orphaned)} entr(y/ies) in examples.yml have no file on disk:")
        print("\n".join(f"  - {entry['name']}: {entry['path']}" for entry in orphaned))
        failures.extend(f"{entry['name']} ({entry['path']}): file not found on disk" for entry in orphaned)
    if not missing and not orphaned:
        print("All example files are registered and all registered paths exist.")

    _section("Check 2: required fields")
    field_problems = list(registry.check_required_fields())
    _report(field_problems, "All entries have the required fields.")
    failures.extend(field_problems)

    _section("Check 3: helper_scripts, data_files and post_script paths")
    path_problems = list(registry.check_auxiliary_paths())
    _report(path_problems, "All auxiliary paths resolved.")
    failures.extend(path_problems)

    _section(f"FAILED: {len(failures)} issue(s) found" if failures else "OK: examples.yml is in sync")
    if failures:
        print("\n".join(f"  - {failure}" for failure in failures))
        print("\nUpdate pynest/examples/examples.yml, commit, and run this check again.")
        return 1
    return 0


if __name__ == "__main__":
    # Only the command line turns a broken registry into an exit code; the
    # functions above raise so that callers can handle failure differently.
    try:
        sys.exit(main())
    except (OSError, ValueError, yaml.YAMLError) as error:
        logger.error("%s", error)
        sys.exit(1)
