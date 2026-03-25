#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# sync_examples.py
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
YAML synchronization script for NEST examples.

This script:
1. Scans pynest/examples/ for Python files and subdirectories
2. Compares discovered examples with examples.yml
3. Reports missing entries (new files not in YAML)
4. Reports orphaned entries (files in YAML but don't exist)
5. Provides suggested YAML additions for review

Usage:
    python sync_examples.py --examples-dir pynest/examples
    python sync_examples.py --examples-dir pynest/examples --output suggestions.yml

Output includes:
- List of missing entries (new files to add)
- List of orphaned entries (files to remove from YAML)
- Suggested YAML additions (copy-paste ready)
"""

import argparse
import os
import sys
from pathlib import Path

import yaml

# Entry point patterns for subdirectories
ENTRY_POINTS = [
    "run_simulation.py",
    "run_simulations.py",
    "sudoku_solver.py",
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


def get_entry_point_files(directory):
    """
    Find main entry point files in a directory.

    Priority:
    1. Known entry point patterns (run_simulation.py, etc.)
    2. Single .py file if only one exists
    3. Empty list if ambiguous or none found
    """
    if not directory.is_dir():
        return []

    py_files = []
    for f in directory.iterdir():
        if f.is_file() and f.suffix == ".py":
            name = f.name
            if not should_ignore(name):
                py_files.append(f)

    if not py_files:
        return []

    for pattern in ENTRY_POINTS:
        matching = [f for f in py_files if f.name == pattern]
        if matching:
            return matching

    if len(py_files) == 1:
        return py_files

    return py_files


def should_ignore(filename):
    """Check if a file/directory should be ignored."""
    for pattern in IGNORE_PATTERNS:
        if pattern.startswith("*"):
            if filename.endswith(pattern[1:]):
                return True
        else:
            if filename == pattern:
                return True
    return False


def is_snakemake_project(directory):
    """Check if a directory is a Snakemake project."""
    return (directory / "Snakefile").exists() or (directory / ".snakemake").is_dir()


def scan_examples_directory(examples_dir):
    """
    Scan examples directory for Python files and subdirectories.

    Returns:
        dict: Mapping of (category, name) -> paths list
    """
    discovered = {}

    for item in examples_dir.iterdir():
        if item.is_file() and item.suffix == ".py" and not should_ignore(item.name):
            name = item.stem
            category = "other"
            key = (category, name)
            if key not in discovered:
                discovered[key] = [str(item.relative_to(examples_dir))]

    for item in examples_dir.iterdir():
        if item.is_dir() and not should_ignore(item.name):
            if is_snakemake_project(item):
                continue

            entry_points = get_entry_point_files(item)
            if entry_points:
                for ep in entry_points:
                    rel_path = str(ep.relative_to(examples_dir))
                    base_name = ep.stem if ep.name not in ENTRY_POINTS else item.name
                    category = item.name.lower()
                    key = (category, base_name)
                    if key not in discovered:
                        discovered[key] = [rel_path]

    return discovered


def load_yaml_file(yaml_file):
    """Load and parse examples.yml."""
    if not yaml_file.exists():
        print(f"Error: {yaml_file} not found", file=sys.stderr)
        sys.exit(1)

    try:
        with open(yaml_file, "r") as f:
            data = yaml.safe_load(f)
        return data
    except yaml.YAMLError as e:
        print(f"Error: Invalid YAML file: {e}", file=sys.stderr)
        sys.exit(1)


def validate_yaml(data):
    """Validate YAML structure."""
    if not isinstance(data, dict):
        print("Error: YAML root must be a dictionary", file=sys.stderr)
        sys.exit(1)

    if "examples" not in data:
        print("Error: YAML must contain 'examples' key", file=sys.stderr)
        sys.exit(1)

    if not isinstance(data["examples"], list):
        print("Error: 'examples' must be a list", file=sys.stderr)
        sys.exit(1)

    for i, example in enumerate(data["examples"]):
        if not isinstance(example, dict):
            print(f"Error: Example {i} must be a dictionary", file=sys.stderr)
            sys.exit(1)

        if "name" not in example:
            print(f"Error: Example {i} missing 'name' field", file=sys.stderr)
            sys.exit(1)

        if "path" not in example:
            print(f"Error: Example '{example.get('name')}' missing 'path' field", file=sys.stderr)
            sys.exit(1)


def compare_discovered_with_yaml(discovered, yaml_data, examples_dir):
    """
    Compare discovered examples with YAML entries.

    Returns:
        tuple: (missing, orphaned, in_yaml)
    """
    yaml_entries = {}
    for example in yaml_data["examples"]:
        name = example["name"]
        path = example["path"]
        yaml_entries[path] = name

    missing = []
    orphaned = []

    for (category, name), paths in discovered.items():
        for path in paths:
            if path not in yaml_entries:
                missing.append({"category": category, "name": name, "path": path})

    for path, name in yaml_entries.items():
        full_path = examples_dir / path
        if not full_path.exists():
            orphaned.append({"name": name, "path": path})

    in_yaml = set(yaml_entries.keys()) & set(p for paths in discovered.values() for p in paths)

    return missing, orphaned, in_yaml


def generate_suggested_yaml(missing):
    """Generate YAML snippets for missing entries."""
    if not missing:
        return None

    suggested = []
    current_category = None

    for item in sorted(missing, key=lambda x: (x["category"], x["name"])):
        if item["category"] != current_category:
            if current_category is not None:
                suggested.append("")
            suggested.append(f"  # {item['category'].capitalize()} examples")
            current_category = item["category"]

        suggested.append(f"  - name: {item['name']}")
        suggested.append(f"    path: {item['path']}")
        suggested.append(f"    type: python")
        suggested.append(f"    convert_to_notebook: true")
        suggested.append(f"    category: {item['category']}")
        suggested.append("")

    return "\n".join(suggested)


def main():
    parser = argparse.ArgumentParser(description="Sync examples.yml with pynest/examples/ directory")
    parser.add_argument("--examples-dir", required=True, help="Path to pynest/examples/ directory")
    parser.add_argument("--output", type=str, help="Output file for suggested YAML additions")

    args = parser.parse_args()

    examples_dir = Path(args.examples_dir).resolve()
    yaml_file = examples_dir / "examples.yml"

    try:
        yaml_data = load_yaml_file(yaml_file)
        validate_yaml(yaml_data)
        print(f"✓ YAML file is valid: {yaml_file}")
    except SystemExit as e:
        if e.code != 0:
            raise

    print("\nScanning examples directory...")
    discovered = scan_examples_directory(examples_dir)
    print(f"  Found {len(discovered)} examples")

    print("\nComparing with examples.yml...")
    missing, orphaned, in_yaml = compare_discovered_with_yaml(discovered, yaml_data, examples_dir)

    print(f"\n{'='*60}")
    print(f"MISSING ENTRIES ({len(missing)} new files to add)")
    print(f"{'='*60}")
    if missing:
        for item in sorted(missing, key=lambda x: (x["category"], x["name"])):
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
        suggested = generate_suggested_yaml(missing)
        print(suggested)
        if args.output:
            with open(args.output, "w") as f:
                f.write(suggested)
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
