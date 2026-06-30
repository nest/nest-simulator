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
Registry validation script for NEST examples.

Validates that pynest/examples/ and examples.yml are in sync — it does not
modify examples.yml.  Run this before prerelease_check.py.

This script:
1. Scans pynest/examples/ for Python files and subdirectories
2. Compares discovered examples with examples.yml
3. Reports missing entries (new files not in YAML)
4. Reports orphaned entries (files in YAML but don't exist)
5. Provides suggested YAML additions for review

Usage:
    python check_examples_registry.py --examples-dir pynest/examples
    python check_examples_registry.py --examples-dir pynest/examples --output suggestions.yml
    python check_examples_registry.py --examples-dir pynest/examples --next-version v3.10

Output includes:
- List of missing entries (new files to add)
- List of orphaned entries (files to remove from YAML)
- Suggested YAML additions (copy-paste ready)
"""

import argparse
import fnmatch
import logging
import sys
import textwrap
from pathlib import Path

import yaml

logger = logging.getLogger(__name__)

# Generic entry-point filename conventions, used *only* to guess the main
# script of a brand-new example directory when generating suggested YAML stubs.
# Validation does not rely on this list: examples.yml is the source of truth for
# which files are entry points (path), helpers (helper_scripts), and post-run
# scripts (post_script).  Keep this list generic — do not add example-specific
# filenames here.
ENTRY_POINTS = [
    "run_simulation.py",
    "run_simulations.py",
    "main.py",
    "__main__.py",
]

# Files and directories to ignore
IGNORE_PATTERNS = [
    "__pycache__",
    ".mypy_cache",
    ".ipynb_checkpoints",
    ".git",
    ".snakemake",
    "example_logs",
    "__init__.py",
    "README.rst",
    "README.txt",
    "CMakeLists.txt",
    "DEPENDENCIES.md",
    "Snakefile",
    "*.svg",
    "*.png",
    "*.dat",
    "*.pkl",
    "*.yml",
    "*.yaml",
]


def should_ignore(name: str) -> bool:
    return any(fnmatch.fnmatch(name, pattern) for pattern in IGNORE_PATTERNS)


def is_snakemake_project(directory: Path) -> bool:
    # Detect on Snakefile only; .snakemake/ is a local runtime artifact and is
    # gitignored, so it cannot be a reliable signal across machines.
    return (directory / "Snakefile").is_file()


def guess_entry_point(py_files: list[Path]) -> Path | None:
    """Best-effort guess of a directory's main script, for suggestion stubs only.

    Used to give a newly discovered example directory a sensible name in the
    suggested-YAML output.  It is *not* used for validation — examples.yml is
    the source of truth for entry points.

    Returns the guessed entry-point Path, or None when it is ambiguous (no
    conventional filename and more than one .py file), in which case each file
    is suggested under its own stem.
    """
    for pattern in ENTRY_POINTS:
        for path in py_files:
            if path.name == pattern:
                return path
    if len(py_files) == 1:
        return py_files[0]
    return None


def scan_examples_directory(examples_dir: Path) -> dict[tuple[str, str], list[str]]:
    """Scan examples directory for every Python file on disk.

    All .py files (top-level and inside subdirectories) are reported, including
    helpers and post-run scripts.  Distinguishing entry points from helpers is
    examples.yml's job; compare_discovered_with_yaml() filters out anything the
    YAML already accounts for.

    Returns:
        Mapping of (category, name) -> list of relative paths
    """
    discovered: dict[tuple[str, str], list[str]] = {}

    for item in examples_dir.iterdir():
        if item.is_file() and item.suffix == ".py" and not should_ignore(item.name):
            key = ("other", item.stem)
            discovered.setdefault(key, []).append(str(item.relative_to(examples_dir)))
        elif item.is_dir() and not should_ignore(item.name) and not is_snakemake_project(item):
            py_files = [
                path
                for path in item.iterdir()
                if path.is_file() and path.suffix == ".py" and not should_ignore(path.name)
            ]
            entry_point = guess_entry_point(py_files)
            for path in py_files:
                rel_path = str(path.relative_to(examples_dir))
                base_name = item.name if path == entry_point else path.stem
                key = (item.name.lower(), base_name)
                discovered.setdefault(key, []).append(rel_path)

    return discovered


def load_yaml_file(yaml_file: Path) -> dict:
    """Load and parse examples.yml."""
    if not yaml_file.exists():
        logger.error("%s not found", yaml_file)
        sys.exit(1)
    try:
        return yaml.safe_load(yaml_file.read_text(encoding="utf-8"))
    except yaml.YAMLError as exc:
        logger.error("Invalid YAML file: %s", exc)
        sys.exit(1)


def validate_yaml(data: dict) -> None:
    """Validate YAML structure."""
    if not isinstance(data, dict):
        logger.error("YAML root must be a dictionary")
        sys.exit(1)
    if "examples" not in data:
        logger.error("YAML must contain 'examples' key")
        sys.exit(1)
    if not isinstance(data["examples"], list):
        logger.error("'examples' must be a list")
        sys.exit(1)
    seen_paths: dict[str, str] = {}
    for idx, example in enumerate(data["examples"]):
        if not isinstance(example, dict):
            logger.error("Example %d must be a dictionary", idx)
            sys.exit(1)
        if "name" not in example:
            logger.error("Example %d missing 'name' field", idx)
            sys.exit(1)
        if "path" not in example:
            logger.error("Example '%s' missing 'path' field", example.get("name"))
            sys.exit(1)
        path = example["path"]
        if path in seen_paths:
            logger.error(
                "Duplicate path %r in examples.yml (entries %r and %r)",
                path,
                seen_paths[path],
                example["name"],
            )
            sys.exit(1)
        seen_paths[path] = example["name"]


def compare_discovered_with_yaml(
    discovered: dict[tuple[str, str], list[str]],
    yaml_data: dict,
    examples_dir: Path,
) -> tuple[list[dict], list[dict]]:
    """Compare discovered examples with YAML entries.

    A .py file on disk is considered registered if examples.yml references it in
    any role — as an entry-point ``path``, a ``helper_scripts`` entry, or a
    ``post_script``.  This makes the missing-file check correct by construction
    (no guessing which file is the entry point) and uniform across single- and
    multi-file examples.

    Returns:
        (missing, orphaned) where missing are files not referenced by YAML and
        orphaned are entry-point paths whose files don't exist on disk.
    """
    referenced: set[str] = set()
    for example in yaml_data["examples"]:
        referenced.add(example["path"])
        referenced.update(example.get("helper_scripts", []))
        if example.get("post_script"):
            referenced.add(example["post_script"])

    missing = [
        {"category": category, "name": name, "path": path}
        for (category, name), paths in discovered.items()
        for path in paths
        if path not in referenced
    ]
    orphaned = [
        {"name": example["name"], "path": example["path"]}
        for example in yaml_data["examples"]
        if not (examples_dir / example["path"]).exists()
    ]

    return missing, orphaned


def generate_suggested_yaml(missing: list[dict], since_version: str | None = None) -> str | None:
    """Generate YAML snippets for missing entries.

    Args:
        missing: List of missing entry dicts with 'category', 'name', 'path'.
        since_version: Upcoming release tag (e.g. "v3.10").  When provided,
            the suggested last_change uses "new since version <tag>" so the
            contributor does not need to edit it manually.  When omitted,
            "no change" is used as a placeholder.
    """
    if not missing:
        return None

    last_change_value = f"new since version {since_version}" if since_version else "no change"

    chunks: list[str] = []
    current_category: str | None = None

    for item in sorted(missing, key=lambda entry: (entry["category"], entry["name"])):
        if item["category"] != current_category:
            if current_category is not None:
                chunks.append("")
            chunks.append(f"  # {item['category'].capitalize()} examples")
            current_category = item["category"]
        entry = {
            "name": item["name"],
            "path": item["path"],
            "runner": "python",
            "run_in_ci": True,
            "convert_to_notebook": True,
            "category": item["category"],
            "last_change": last_change_value,
        }
        # safe_dump handles quoting for any special characters in name/path/category.
        rendered = yaml.safe_dump([entry], default_flow_style=False, sort_keys=False).rstrip()
        chunks.append(textwrap.indent(rendered, "  "))

    return "\n".join(chunks) + "\n"


def main() -> None:
    logging.basicConfig(format="%(levelname)s: %(message)s", level=logging.WARNING)

    parser = argparse.ArgumentParser(description="Validate examples.yml registry against pynest/examples/ directory")
    parser.add_argument("--examples-dir", required=True, help="Path to pynest/examples/ directory")
    parser.add_argument("--output", type=str, help="Output file for suggested YAML additions")
    parser.add_argument(
        "--next-version",
        type=str,
        default=None,
        metavar="TAG",
        help="Upcoming release tag (e.g. v3.10). When set, suggested stubs use "
        "'new since version <TAG>' instead of 'no change'.",
    )

    args = parser.parse_args()

    examples_dir = Path(args.examples_dir).resolve()
    yaml_file = examples_dir / "examples.yml"

    yaml_data = load_yaml_file(yaml_file)
    validate_yaml(yaml_data)
    print(f"✓ YAML file is valid: {yaml_file}")

    print("\nScanning examples directory...")
    discovered = scan_examples_directory(examples_dir)
    print(f"  Found {len(discovered)} examples")

    print("\nComparing with examples.yml...")
    missing, orphaned = compare_discovered_with_yaml(discovered, yaml_data, examples_dir)

    print(f"\n{'='*60}")
    print(f"MISSING ENTRIES ({len(missing)} new files to add)")
    print(f"{'='*60}")
    if missing:
        for item in sorted(missing, key=lambda entry: (entry["category"], entry["name"])):
            print(f"  - {item['name']}: {item['path']}")
    else:
        print("  No missing entries found.")

    print(f"\n{'='*60}")
    print(f"ORPHANED ENTRIES ({len(orphaned)} entries to review/remove)")
    print(f"{'='*60}")
    if orphaned:
        for item in orphaned:
            print(f"  ⚠ {item['name']}: {item['path']} (file not found)")
    else:
        print("  No orphaned entries found.")

    print(f"\n{'='*60}")
    print("SUGGESTED YAML ADDITIONS")
    print(f"{'='*60}")
    if missing:
        suggested = generate_suggested_yaml(missing, since_version=args.next_version)
        print(suggested)
        if args.output:
            with open(args.output, "w") as output_file:
                output_file.write(suggested)
            print(f"\n✓ Suggestions written to: {args.output}")
    else:
        print("No additions needed.")

    print(f"\n{'='*60}")
    if missing or orphaned:
        print("FAILED: examples.yml is out of sync with pynest/examples/.")
        print("Add missing entries to examples.yml and remove orphaned ones.")
        print(f"{'='*60}")
        sys.exit(1)
    print("OK: examples.yml is in sync with pynest/examples/.")
    print(f"{'='*60}")


if __name__ == "__main__":
    main()
