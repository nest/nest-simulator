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

Run this after sync_examples.py (which checks file/YAML sync).
This script checks:

  1. Changed files: any example file changed since PREVIOUS_TAG must have
     its last_change field updated from "no change".
  2. New files: any example file that did not exist in PREVIOUS_TAG must have
     last_change starting with "new since".
  3. Required fields: all YAML entries must have non-empty values for every
     required field.
  4. Paths: all helper_scripts and data_files paths must exist on disk.

If all checks pass, exit 0 — safe to trigger downstream dispatch.
If any check fails, exit 1 — fix examples.yml and re-run.

Usage:
    python prerelease_check.py --examples-dir pynest/examples --previous-tag vMAJOR.MINOR
    python prerelease_check.py --examples-dir pynest/examples --previous-tag vMAJOR.MINOR --current-tag vMAJOR.MINOR-rcN
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

import yaml

# Accepted tag formats: vMAJOR.MINOR  vMAJOR.MINOR-rcN  vMAJOR.MINOR.PATCH
_TAG_RE = re.compile(r"^v[0-9]+\.[0-9]+(?:[.\-][\w.]+)?$")

REQUIRED_FIELDS = ["name", "path", "type", "category", "convert_to_notebook", "last_change"]

NO_CHANGE = "no change"


def get_repo_root():
    result = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        capture_output=True,
        text=True,
        check=True,
    )
    return Path(result.stdout.strip())


def run_git(args, repo_root):
    return subprocess.run(
        ["git"] + args,
        cwd=repo_root,
        capture_output=True,
        text=True,
        check=True,
    )


def get_changed_files(prev_tag, examples_dir, repo_root):
    """
    Return a list of paths relative to examples_dir that changed since prev_tag.
    Only includes .py files.
    """
    rel_examples = examples_dir.relative_to(repo_root)
    result = run_git(["diff", "--name-only", f"{prev_tag}..HEAD", "--", str(rel_examples)], repo_root)
    prefix = str(rel_examples) + "/"
    changed = []
    for line in result.stdout.strip().splitlines():
        line = line.strip()
        if line and line.endswith(".py"):
            changed.append(line.removeprefix(prefix))
    return changed


def existed_in_tag(rel_path, tag, examples_dir, repo_root):
    """Return True if examples_dir/rel_path existed in the given tag."""
    rel_examples = examples_dir.relative_to(repo_root)
    result = subprocess.run(
        ["git", "ls-tree", "--name-only", tag, f"{rel_examples}/{rel_path}"],
        cwd=repo_root,
        capture_output=True,
        text=True,
    )
    return bool(result.stdout.strip())


def load_entries(examples_dir):
    yaml_file = examples_dir / "examples.yml"
    with open(yaml_file) as f:
        data = yaml.safe_load(f)
    return data["examples"]


def main():
    parser = argparse.ArgumentParser(description="Pre-release validation for examples.yml")
    parser.add_argument("--examples-dir", required=True, help="Path to pynest/examples/ directory")
    parser.add_argument("--previous-tag", required=True, help="Last stable release tag, e.g. vMAJOR.MINOR")
    parser.add_argument("--current-tag", default="HEAD", help="Current tag or ref (default: HEAD)")
    args = parser.parse_args()

    examples_dir = Path(args.examples_dir).resolve()
    prev_tag = args.previous_tag
    current_tag = args.current_tag
    repo_root = get_repo_root()

    print(f"Examples directory       : {examples_dir}")
    print(f"Previous stable release  : {prev_tag}")
    print(f"Current ref              : {current_tag}")
    print()

    entries = load_entries(examples_dir)
    by_path = {e["path"]: e for e in entries}
    failures = []

    # ── Check 1+2: changed and new files ─────────────────────────────────────
    print(f"Checking for changes in pynest/examples/ since {prev_tag}...")
    changed_files = get_changed_files(prev_tag, examples_dir, repo_root)

    if changed_files:
        print(f"  {len(changed_files)} .py file(s) changed:")
        for f in sorted(changed_files):
            print(f"    {f}")
    else:
        print(f"  No .py files changed since {prev_tag}.")

    for rel_path in sorted(changed_files):
        entry = by_path.get(rel_path)
        if entry is None:
            # Not registered in YAML — sync_examples.py catches this.
            continue

        name = entry.get("name", rel_path)
        last_change = entry.get("last_change") or ""
        is_new = not existed_in_tag(rel_path, prev_tag, examples_dir, repo_root)

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
        if entry.get("convert_to_notebook") is True and not entry.get("models"):
            field_failures.append(f"  {name}: convert_to_notebook=true but 'models' is missing or empty")

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
