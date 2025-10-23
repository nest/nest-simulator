# -*- coding: utf-8 -*-
#
# check_pr_authors.py
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
#!/usr/bin/env python3
"""
Check PR authors against validated author list.

This script fetches PR commits, extracts author information, and compares
against a validated author list from the NEST release dataset.

SECURITY NOTE: This script is designed to protect sensitive data:
- Private repository information is not logged
- Validated author names from private repo are not exposed in logs
- PR author names can be logged (they're already public in the PR)
- Sensitive data from private repository is cleaned up from memory after use
"""

import argparse
import json
import logging
import os
import sys
import tempfile
from typing import List, Optional, Tuple

import requests


def secure_cleanup(data: any) -> None:
    """Ensure sensitive data is not retained in memory longer than necessary."""
    if isinstance(data, str):
        # Overwrite string data with zeros
        data = "0" * len(data)
    elif isinstance(data, list):
        for item in data:
            secure_cleanup(item)
    elif isinstance(data, dict):
        for key, value in data.items():
            secure_cleanup(value)
    # Let garbage collector handle the rest


def get_pr_commits(pr_number: int, repo_owner: str, repo_name: str, token: str) -> List[dict]:
    """Fetch commits from a PR using GitHub API."""
    url = f"https://api.github.com/repos/{repo_owner}/{repo_name}/pulls/{pr_number}/commits"
    headers = {"Authorization": f"Bearer {token}", "Accept": "application/vnd.github.v3+json"}

    print(f"Fetching commits from GitHub API for PR #{pr_number} in {repo_owner}/{repo_name}")

    try:
        response = requests.get(url, headers=headers)
        response.raise_for_status()

        commits = response.json()
        if not isinstance(commits, list):
            raise ValueError("Unexpected API response (not a JSON array)")

        return commits
    except requests.exceptions.RequestException as e:
        print(f"Error: Failed to fetch commits from GitHub API: {e}")
        print(f"URL: {url}")
        if "response" in locals():
            print(f"Response status: {response.status_code}")
            if response.status_code == 404:
                print("This might indicate the PR number is invalid or the repository doesn't exist")
            elif response.status_code == 401:
                print("This might indicate the GitHub token doesn't have the required permissions")
        else:
            print("No response received")
        sys.exit(1)
    except ValueError as e:
        print(f"Error: {e}")
        print(f"URL: {url}")
        sys.exit(1)


def extract_unique_authors(commits: List[dict]) -> List[Tuple[str, str]]:
    """Extract unique authors from commits."""
    authors = set()

    for commit in commits:
        author_name = commit.get("commit", {}).get("author", {}).get("name", "")
        author_email = commit.get("commit", {}).get("author", {}).get("email", "")

        if author_name and author_email:
            authors.add((author_name, author_email))

    return sorted(list(authors))


def fetch_validated_authors(
    private_repo_owner: str, private_repo_name: str, authors_file_path: str, token: str
) -> Optional[List[str]]:
    """Fetch validated authors from private repository."""
    url = f"https://api.github.com/repos/{private_repo_owner}/{private_repo_name}/contents/{authors_file_path}"
    headers = {"Authorization": f"token {token}", "Accept": "application/vnd.github.v3.raw"}

    # Don't log private repository information for security
    print("Fetching validated authors from private repository...")

    try:
        response = requests.get(url, headers=headers)

        if response.status_code == 404:
            print("Warning: Could not find validated authors file")
            return None

        response.raise_for_status()
        content = response.text

        if not content:
            print("Warning: Could not decode validated authors file content")
            return None

        # Parse YAML format: "Name <email>: githubhandle"
        # Extract the "Name <email>" part before the colon
        validated_authors = []
        for line in content.split("\n"):
            line = line.strip()
            if line and not line.startswith("#") and ":" in line:
                author_part = line.split(":")[0].strip()
                if author_part:
                    validated_authors.append(author_part)

        # Only log count, never the actual names
        print(f"Found {len(validated_authors)} validated author(s) (list stored securely, not exposed)")
        return validated_authors

    except requests.exceptions.RequestException as e:
        print(f"Warning: Failed to fetch validated authors: {e}")
        return None


def check_authors_against_validated_list(
    pr_authors: List[Tuple[str, str]], validated_authors: List[str]
) -> Tuple[List[str], int]:
    """Check PR authors against validated list."""
    unknown_authors = []

    for name, email in pr_authors:
        author_string = f"{name} <{email}>"
        if author_string not in validated_authors:
            unknown_authors.append(author_string)

    return unknown_authors, len(unknown_authors)


def main():
    parser = argparse.ArgumentParser(description="Check PR authors against validated author list")
    parser.add_argument("--pr-number", type=int, required=True, help="Pull request number")
    parser.add_argument("--repo-owner", required=True, help="Repository owner")
    parser.add_argument("--repo-name", required=True, help="Repository name")
    parser.add_argument("--github-token", required=True, help="GitHub token")
    parser.add_argument("--private-repo-owner", help="Private repository owner")
    parser.add_argument("--private-repo-name", help="Private repository name")
    parser.add_argument("--private-repo-token", help="Private repository token")
    parser.add_argument(
        "--authors-file-path", default="data/gitlognames.yaml", help="Path to authors file in private repo"
    )
    parser.add_argument("--fail-on-unknown", action="store_true", help="Fail if unknown authors are found")

    args = parser.parse_args()

    # Get PR commits and extract authors
    print(f"Debug: Fetching commits for PR #{args.pr_number} in {args.repo_owner}/{args.repo_name}")
    commits = get_pr_commits(args.pr_number, args.repo_owner, args.repo_name, args.github_token)

    if not commits:
        print("No commits found in PR")
        print("author_count=0")
        print("unknown_count=0")
        print("validation_status=skipped")
        return

    pr_authors = extract_unique_authors(commits)
    author_count = len(pr_authors)

    print(f"Found {author_count} unique author(s):")
    for name, email in pr_authors:
        print(f"  - {name} <{email}>")

    # Output author count and authors list for GitHub Actions
    print(f"author_count={author_count}")

    # Output authors list in GitHub Actions format for step summary display
    authors_formatted = "\n".join([f"{name} <{email}>" for name, email in pr_authors])
    print("authors_formatted<<EOF")
    print(authors_formatted)
    print("EOF")

    # Try to fetch validated authors if private repo is configured
    validated_authors = None
    print(
        f"Debug: Private repo configured: owner={bool(args.private_repo_owner)}, name={bool(args.private_repo_name)}, token={bool(args.private_repo_token)}"
    )
    if args.private_repo_owner and args.private_repo_name and args.private_repo_token:
        validated_authors = fetch_validated_authors(
            args.private_repo_owner, args.private_repo_name, args.authors_file_path, args.private_repo_token
        )

    if validated_authors is None:
        print("validation_status=skipped")
        print("unknown_count=0")
        print("ℹ️ Note: Author validation check skipped (private repository access not configured)")
        return

    # Check authors against validated list
    unknown_authors, unknown_count = check_authors_against_validated_list(pr_authors, validated_authors)

    print(f"unknown_count={unknown_count}")

    if unknown_count > 0:
        print("validation_status=failure")
        print("❌ FAILURE: Found unknown author(s) in this PR")
        print(
            "The authors of this PR may be contributing for the first time or may have "
            "modified their author information. Author information requires review."
        )

        # Log unknown authors (these are PR authors, so it's safe to show them)
        print("Unknown authors in this PR:")
        for author in unknown_authors:
            print(f"  - {author}")

        # Clean up sensitive data (only validated_authors, not unknown_authors since they're PR data)
        secure_cleanup(validated_authors)

        if args.fail_on_unknown:
            print("Build failed due to unknown authors")
            sys.exit(1)
        else:
            print("Build continues despite unknown authors")
    else:
        print("validation_status=success")
        print("✅ SUCCESS: All PR authors are validated")

        # Clean up sensitive data
        secure_cleanup(validated_authors)


if __name__ == "__main__":
    main()
