#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# prerelease_check.py
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
Pre-release validation for pynest/examples/examples.yml.

Run this after check_examples_registry.py (which checks file/YAML sync).
This script checks:

  2. Changed files: any example file changed since the previous stable tag must
     have its last_change field updated from "no change".
     New files must have last_change starting with "new since".
  3. Required fields: all YAML entries must have non-empty values for every
     required field.  Entries with run_in_ci: false must have a non-empty
     notes field.  runner must be one of the allowed values.
  4. Paths: all helper_scripts and data_files paths must exist on disk.

Check numbers correspond to the steps in trigger-examples-on-prerelease.yml.
Check 1 (registry sync) is performed by check_examples_registry.py.

If all checks pass, exit 0 — safe to trigger downstream dispatch.
If any check fails, exit 1 — fix examples.yml and re-run.

Usage:
    python prerelease_check.py --examples-dir pynest/examples --previous-tag vMAJOR.MINOR
"""

import argparse
import logging
import subprocess
import sys
from pathlib import Path

import yaml

logger = logging.getLogger(__name__)

REQUIRED_FIELDS = ["name", "path", "runner", "run_in_ci", "category", "convert_to_notebook", "last_change"]

RUNNER_VALUES = {"python", "snakemake"}

NO_CHANGE = "no change"


def get_repo_root() -> Path:
    result = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        capture_output=True,
        text=True,
        check=True,
    )
    return Path(result.stdout.strip())


def run_git(git_args: list[str], repo_root: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git"] + git_args,
        cwd=repo_root,
        capture_output=True,
        text=True,
        check=True,
    )


def get_changed_files(prev_tag: str, examples_dir: Path, repo_root: Path) -> list[str]:
    """Return paths relative to examples_dir that changed since prev_tag (only .py files)."""
    rel_examples = examples_dir.relative_to(repo_root)
    result = run_git(["diff", "--name-only", f"{prev_tag}..HEAD", "--", str(rel_examples)], repo_root)
    prefix = str(rel_examples) + "/"
    stripped_lines = [line.strip() for line in result.stdout.splitlines()]
    return [line.removeprefix(prefix) for line in stripped_lines if line.endswith(".py")]


def exists_at_tag(rel_path: str, tag: str, examples_dir: Path, repo_root: Path) -> bool:
    """Return True if examples_dir/rel_path existed in the given tag."""
    rel_examples = examples_dir.relative_to(repo_root)
    result = subprocess.run(
        ["git", "ls-tree", "--name-only", tag, f"{rel_examples}/{rel_path}"],
        cwd=repo_root,
        capture_output=True,
        text=True,
        check=True,
    )
    return bool(result.stdout.strip())


def validate_tag(tag: str, repo_root: Path) -> None:
    """Exit with a friendly error if tag does not resolve to a commit."""
    result = subprocess.run(
        ["git", "rev-parse", "--verify", f"{tag}^{{commit}}"],
        cwd=repo_root,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        logger.error("git tag %r not found. Fetch tags with: git fetch --tags --force", tag)
        sys.exit(1)


def load_yaml_entries(examples_dir: Path) -> list[dict]:
    yaml_file = examples_dir / "examples.yml"
    with yaml_file.open(encoding="utf-8") as yaml_fh:
        data = yaml.safe_load(yaml_fh)
    return data["examples"]


def main() -> None:
    logging.basicConfig(format="%(levelname)s: %(message)s", level=logging.WARNING)

    parser = argparse.ArgumentParser(description="Pre-release validation for examples.yml")
    parser.add_argument("--examples-dir", required=True, help="Path to pynest/examples/ directory")
    parser.add_argument("--previous-tag", required=True, help="Last stable release tag, e.g. vMAJOR.MINOR")
    args = parser.parse_args()

    examples_dir = Path(args.examples_dir).resolve()
    prev_tag = args.previous_tag
    repo_root = get_repo_root()
    validate_tag(prev_tag, repo_root)

    print(f"Examples directory       : {examples_dir}")
    print(f"Previous stable release  : {prev_tag}")
    print()

    entries = load_yaml_entries(examples_dir)
    by_path = {entry["path"]: entry for entry in entries}
    failures = []

    # ── Check 2: changed and new files ───────────────────────────────────────
    print(f"Checking for changes in pynest/examples/ since {prev_tag}...")
    changed_files = get_changed_files(prev_tag, examples_dir, repo_root)

    if changed_files:
        print(f"  {len(changed_files)} .py file(s) changed:")
        for changed_path in sorted(changed_files):
            print(f"    {changed_path}")
    else:
        print(f"  No .py files changed since {prev_tag}.")

    for rel_path in sorted(changed_files):
        entry = by_path.get(rel_path)
        if entry is None:
            # Not registered in YAML — check_examples_registry.py catches this.
            continue

        name = entry.get("name", rel_path)
        last_change = entry.get("last_change") or ""
        is_new = not exists_at_tag(rel_path, prev_tag, examples_dir, repo_root)

        if is_new:
            if not last_change.startswith("new"):
                failures.append(
                    f"  {name} ({rel_path}): new since {prev_tag}, "
                    f"but last_change={last_change!r}. "
                    f'Expected: "new since version {prev_tag}"'
                )
        else:
            if not last_change or last_change == NO_CHANGE:
                failures.append(
                    f"  {name} ({rel_path}): changed since {prev_tag}, "
                    f"but last_change={last_change!r}. "
                    f'Expected: "changes since version {prev_tag}"'
                )

    # ── Check 3: required fields ──────────────────────────────────────────────
    print("\nChecking required fields in all YAML entries...")
    field_failures = []
    for entry in entries:
        name = entry.get("name", "<unnamed>")
        for field in REQUIRED_FIELDS:
            val = entry.get(field)
            if val is None or val == "":
                field_failures.append(f"  {name}: field '{field}' is missing or empty")
        runner = entry.get("runner")
        if runner is not None and runner not in RUNNER_VALUES:
            field_failures.append(f"  {name}: runner={runner!r} not in {sorted(RUNNER_VALUES)}")
        if entry.get("run_in_ci") is False and not entry.get("notes"):
            field_failures.append(f"  {name}: run_in_ci=false but 'notes' is missing or empty (shown in dashboard)")

    if field_failures:
        print(f"  {len(field_failures)} field issue(s) found.")
        failures.extend(field_failures)
    else:
        print("  All required fields present.")

    # ── Check 4: helper_scripts and data_files paths exist on disk ────────────
    print("\nChecking helper_scripts and data_files paths exist on disk...")
    path_failures = []
    for entry in entries:
        name = entry.get("name", "<unnamed>")
        for helper in entry.get("helper_scripts") or []:
            if not (examples_dir / helper).exists():
                path_failures.append(f"  {name}: helper_scripts path not found: {helper}")
        for df in entry.get("data_files") or []:
            loc = df.get("path", "")
            if not loc:
                path_failures.append(f"  {name}: data_files entry missing 'path' key")
                continue
            if not (examples_dir / loc).exists():
                path_failures.append(f"  {name}: data_files path not found: {loc}")

    if path_failures:
        print(f"  {len(path_failures)} path issue(s) found.")
        failures.extend(path_failures)
    else:
        print("  All helper_scripts and data_files paths resolved.")

    # ── Result ────────────────────────────────────────────────────────────────
    print()
    print("=" * 60)
    if failures:
        print(f"FAILED: {len(failures)} issue(s) found in examples.yml.\n")
        for line in failures:
            print(line)
        print("\nUpdate examples.yml, commit, and re-run this workflow.")
        sys.exit(1)

    print("OK: all checks passed. Safe to trigger downstream dispatch.")


if __name__ == "__main__":
    main()
