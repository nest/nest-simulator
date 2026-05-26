#!/usr/bin/env python3
"""
migrate_refs.py -- Convert RST footnote-style references to sphinxcontrib-bibtex footcite roles.

Usage
-----
  # Dry-run (print diff only, no writes):
  python3 migrate_refs.py

  # Apply changes:
  python3 migrate_refs.py --apply

  # Show coverage report only:
  python3 migrate_refs.py --report

What it does
------------
For each C++ header (models/*.h, nestkernel/*.h) and each static RST file:

1. Finds all ``.. [N] Author...`` reference blocks inside BeginUserDocs/EndUserDocs
   (or the full RST file for static files).
2. Matches each block to a BibTeX key in refs.bib by DOI, URL, or author+year.
3. In the text, replaces ``[N]_`` inline citations with ``:footcite:p:`key```.
4. Replaces the entire ``References`` section body with ``.. footbibliography::``.
5. For C++ headers: rewrites the header file.
   For static RST files: rewrites the RST file directly.

References that cannot be matched to refs.bib are left as-is and reported as unmatched.
"""

import argparse
import difflib
import re
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
REPO = Path(__file__).resolve().parent.parent.parent
HTMLDOC = Path(__file__).resolve().parent
BIB_FILE = HTMLDOC / "refs.bib"

MODEL_HEADERS = sorted(list((REPO / "models").glob("*.h")) + list((REPO / "nestkernel").glob("*.h")))

STATIC_RST = [
    HTMLDOC / "hpc/benchmarking.rst",
    HTMLDOC / "hpc/parallel_computing.rst",
    HTMLDOC / "hpc/threading.rst",
    HTMLDOC / "neurons/exact-integration.rst",
    HTMLDOC / "synapses/weight_normalization.rst",
    HTMLDOC / "synapses/simulations_with_gap_junctions.rst",
    HTMLDOC / "synapses/connectivity_concepts.rst",
    HTMLDOC / "synapses/connection_generator.rst",
    HTMLDOC / "interface_nest/using_nest_with_music.rst",
    HTMLDOC / "nest_sonata/nest_sonata_guide.rst",
    HTMLDOC / "tutorials/pynest_tutorial/part_1_neurons_and_simple_neural_networks.rst",
    HTMLDOC / "whats_new/v3.7/index.rst",
    HTMLDOC / "developer_space/guidelines/styleguide/styleguide.rst",
    HTMLDOC / "models/index.rst",
]

# ---------------------------------------------------------------------------
# BibTeX parsing: extract DOI, URL, author, year fields from refs.bib
# ---------------------------------------------------------------------------

doi_field_re = re.compile(r"doi\s*=\s*\{([^}]+)\}", re.IGNORECASE)
url_field_re = re.compile(r"url\s*=\s*\{([^}]+)\}", re.IGNORECASE)
# Handles up to 2 levels of nested braces (e.g. Erd{\H{o}}s)
author_field_re = re.compile(r"author\s*=\s*\{((?:[^{}]|\{(?:[^{}]|\{[^{}]*\})*\})*)\}", re.IGNORECASE)
year_field_re = re.compile(r"year\s*=\s*\{(\d{4})\}", re.IGNORECASE)
key_re = re.compile(r"@\w+\{(\w+),")


def _normalise_doi(doi: str) -> str:
    """Strip protocol prefix and trailing punctuation from a DOI string."""
    doi = doi.strip().rstrip(".,>_`\"'")
    doi = re.sub(r"^https?://(dx\.)?doi\.org/", "", doi, flags=re.IGNORECASE)
    doi = doi.replace("%2F", "/").replace("%3A", ":")
    return doi.lower()


def _normalise_url(url: str) -> str:
    """Normalise a URL for comparison (lower-case, no trailing slashes/punct, no query/fragment)."""
    url = url.strip().rstrip(".,>_`\"'/")
    url = re.sub(r"^http://", "https://", url, flags=re.IGNORECASE)
    # Strip query string and fragment
    url = re.sub(r"[?#].*$", "", url)
    return url.lower()


def _extract_doi_from_url(url: str) -> str | None:
    """Try to extract a DOI from a URL that may embed it (e.g. frontiersin.org)."""
    # DOI pattern in a URL path: 10.XXXX/...
    m = re.search(r"(10\.\d{4,}/\S+)", url)
    if m:
        return _normalise_doi(m.group(1))
    return None


def _strip_bibtex_braces(s: str) -> str:
    """Iteratively remove innermost {…} pairs from a BibTeX string."""
    prev = None
    while prev != s:
        prev = s
        s = re.sub(r"\{[^{}]*\}", "", s)
    return s


def _author_fingerprints(author_raw: str) -> list[tuple]:
    """
    Return list of fingerprint tuples from a BibTeX author field.

    Returns e.g. [("rotter",), ("rotter", "diesmann")] for two-author papers.
    Author field format: "Lastname, Firstname and Lastname, Firstname".
    Also handles "Firstname Lastname" style (all-text entities).
    """
    parts = re.split(r"\s+and\s+", author_raw, flags=re.IGNORECASE)
    last_names = []
    for part in parts:
        part = part.strip()
        if "," in part:
            last = part.split(",")[0]
        else:
            words = part.split()
            last = words[-1] if words else part
        # Strip nested TeX braces ({Erd{\H{o}}s} → Erds), then TeX commands (\H → '')
        last = _strip_bibtex_braces(last)
        last = re.sub(r"\\[a-zA-Z]+\s*", "", last)
        last = re.sub(r"[^a-zA-Z]", "", last)
        last_names.append(last.lower())
    fps = []
    if last_names:
        fps.append((last_names[0],))
        if len(last_names) >= 2:
            fps.append((last_names[0], last_names[1]))
    return fps


def load_bib_index(bib_path: Path) -> tuple[dict, dict]:
    """
    Return two dicts:
      doi_url_index: {normalised_doi_or_url: bibtex_key}
      author_year_index: {(lastname, year) or (last1, last2, year): bibtex_key | None}
                         None means ambiguous (multiple papers match).
    """
    text = bib_path.read_text(encoding="utf-8")
    doi_url_index: dict[str, str] = {}
    author_year_index: dict[tuple, str | None] = {}

    entries = re.split(r"(?=@\w+\{)", text)
    for entry in entries:
        key_m = key_re.search(entry)
        if not key_m:
            continue
        key = key_m.group(1)

        # DOI index
        doi_m = doi_field_re.search(entry)
        if doi_m:
            norm = _normalise_doi(doi_m.group(1))
            doi_url_index[norm] = key

        # URL index (strip query params)
        url_m = url_field_re.search(entry)
        if url_m:
            norm = _normalise_url(url_m.group(1))
            doi_url_index[norm] = key
            # Also index any embedded DOI in the URL
            embedded = _extract_doi_from_url(url_m.group(1))
            if embedded and embedded not in doi_url_index:
                doi_url_index[embedded] = key

        # Author + year index
        author_m = author_field_re.search(entry)
        year_m = year_field_re.search(entry)
        if author_m and year_m:
            year = year_m.group(1)
            for fp in _author_fingerprints(author_m.group(1)):
                fy = fp + (year,)
                if fy not in author_year_index:
                    author_year_index[fy] = key
                elif author_year_index[fy] != key:
                    author_year_index[fy] = None  # ambiguous

    return doi_url_index, author_year_index


# ---------------------------------------------------------------------------
# RST reference parsing
# ---------------------------------------------------------------------------

ref_block_re = re.compile(
    r"(?P<block>^\.\. \[(?P<label>\w+)\](?P<body>.*?))" r"(?=^\.\. \[|\Z)",
    re.MULTILINE | re.DOTALL,
)

inline_doi_re = re.compile(r"https?://(dx\.)?doi\.org/(\S+?)(?:[\s>]|$)", re.IGNORECASE)
inline_url_re = re.compile(r"https?://\S+")

# First author last name: handles "Lastname ..." or "F. Lastname ..." (initial-first)
text_author_re = re.compile(r"^(?:[A-Z]\.?\s+)*([A-Z][a-zÀ-ž\-]+)")
# Year: match any 4-digit year in the 1900-2099 range
text_year_re = re.compile(r"\b((?:19|20)\d{2})\b")
# Second author: comma-separated "Lastname F," or "and Lastname" or "& Lastname"
text_second_author_re = re.compile(
    r"[A-Z][a-zÀ-ž\-]+(?:\s+[A-Z]+\.?,?)+" r"(?:,\s+|(?:\s+(?:and|&)\s+))" r"([A-Z][a-zÀ-ž\-]+)"
)

# Matches the entire References section body — whether it contains raw .. [N] blocks,
# a .. footbibliography:: directive, or a mix from a partial previous run.
references_section_re = re.compile(
    r"(?P<header>References\s*\n[+~\-=^]{3,}\n\n*)" r"(?P<body>.*?)" r"(?=\n[A-Z\*]|EndUserDocs|\Z)",
    re.MULTILINE | re.DOTALL,
)

userdoc_re = re.compile(
    r"(?P<before>BeginUserDocs:[^\n]*\n\n)" r"(?P<doc>.*?)" r"(?P<after>EndUserDocs)",
    re.DOTALL,
)


# Explicit overrides for references whose text doesn't carry a machine-readable DOI/URL
# and whose year in the text differs from the publication year in refs.bib.
# Format: list of (author_regex, year_in_text_or_None, bib_key)
_TEXT_OVERRIDES: list[tuple] = [
    # Morrison et al. 2007 was cited as "2006" while in press
    (re.compile(r"Morrison.*Straube.*Plesser.*Diesmann", re.DOTALL), "2006", "Morrison2007a"),
    # Gerstner & Kistler book — sometimes cited without a year
    (re.compile(r"Gerstner.*Kistler.*Spiking neuron models", re.DOTALL | re.IGNORECASE), None, "Gerstner2002"),
]


def _extract_key_from_body(body: str, doi_url_index: dict, author_year_index: dict) -> str | None:
    """Return a BibTeX key for this reference body, or None if unmatched."""
    # 0. Explicit text-pattern overrides (for edge-case year mismatches / missing years)
    for pattern, year_req, key in _TEXT_OVERRIDES:
        if pattern.search(body):
            if year_req is None or year_req in body:
                return key
    # 1. Try explicit doi.org URL
    doi_m = inline_doi_re.search(body)
    if doi_m:
        norm = _normalise_doi(doi_m.group(0).strip())
        if norm in doi_url_index:
            return doi_url_index[norm]

    # 2. Try any URL (strip query params; also try extracting embedded DOI)
    for url_m in inline_url_re.finditer(body):
        raw = url_m.group(0).strip().rstrip(".,>_`\"'")
        # Try embedded DOI first (handles frontiersin.org, etc.)
        embedded = _extract_doi_from_url(raw)
        if embedded and embedded in doi_url_index:
            return doi_url_index[embedded]
        # Then try normalised URL
        norm = _normalise_url(raw)
        if norm in doi_url_index:
            return doi_url_index[norm]
        # Prefix match: bib URL is a prefix of the text URL (e.g. github repo vs file path)
        for bib_url, key in doi_url_index.items():
            if len(bib_url) > 20 and norm.startswith(bib_url):
                return key
        # Truncated URL match: text URL is a prefix of a bib URL
        if len(norm) > 30:
            for bib_url, key in doi_url_index.items():
                if bib_url.startswith(norm):
                    return key

    # 3. Try author + year text matching (for books and papers without DOI in text)
    year_m = text_year_re.search(body)
    author_m = text_author_re.search(body)
    if year_m and author_m:
        year = year_m.group(1)
        first = re.sub(r"[^a-z]", "", author_m.group(1).lower())

        # Try two-author fingerprint first for disambiguation
        second_m = text_second_author_re.search(body)
        if second_m:
            second = re.sub(r"[^a-z]", "", second_m.group(1).lower())
            fp2 = (first, second, year)
            if fp2 in author_year_index and author_year_index[fp2] is not None:
                return author_year_index[fp2]

        # Single-author fingerprint
        fp1 = (first, year)
        if fp1 in author_year_index and author_year_index[fp1] is not None:
            return author_year_index[fp1]

    return None


def migrate_doc_text(doc: str, doi_url_index: dict, author_year_index: dict) -> tuple[str, dict, list]:
    """
    Convert all .. [N] reference blocks in a RST doc string to :footcite: roles.

    Returns
    -------
    new_doc : str
    label_to_key : dict[label, bib_key]
    unmatched : list[(label, body)]
    """
    label_to_key: dict[str, str] = {}
    unmatched: list = []

    for m in ref_block_re.finditer(doc):
        label = m.group("label")
        body = m.group("body")
        body_flat = re.sub(r"\n\s+", " ", body).strip()
        key = _extract_key_from_body(body_flat, doi_url_index, author_year_index)
        if key:
            label_to_key[label] = key
        else:
            unmatched.append((label, body_flat))

    if not label_to_key:
        return doc, label_to_key, unmatched

    # Replace inline [N]_ citations with :footcite:p:`key`
    matched_labels = "|".join(re.escape(l) for l in label_to_key)
    inline_ref_re = re.compile(r"\[(" + matched_labels + r")\]_")

    def replace_inline(m):
        label = m.group(1)
        return f":footcite:p:`{label_to_key[label]}`"

    new_doc = inline_ref_re.sub(replace_inline, doc)

    # Replace the References section body: keep only unmatched .. [N] entries,
    # and ensure .. footbibliography:: is present.
    def replace_refs_section(m):
        header = m.group("header")
        body = m.group("body")
        # Find any .. [N] blocks remaining in the body (matched or not)
        all_blocks = list(ref_block_re.finditer(body))
        remaining = [b for b in all_blocks if b.group("label") not in label_to_key]
        if remaining:
            remaining_text = "".join(b.group("block") for b in remaining)
            return header + ".. footbibliography::\n\n" + remaining_text
        else:
            return header + ".. footbibliography::\n"

    new_doc = references_section_re.sub(replace_refs_section, new_doc)
    return new_doc, label_to_key, unmatched


# ---------------------------------------------------------------------------
# Per-file processing
# ---------------------------------------------------------------------------


def process_header(path: Path, doi_url_index: dict, author_year_index: dict) -> tuple[str | None, dict, list]:
    original = path.read_text(encoding="utf-8")
    m = userdoc_re.search(original)
    if not m:
        return None, {}, []
    doc = m.group("doc")
    new_doc, l2k, unmatched = migrate_doc_text(doc, doi_url_index, author_year_index)
    if new_doc == doc:
        return None, l2k, unmatched
    new_content = original[: m.start("doc")] + new_doc + original[m.end("doc") :]
    return new_content, l2k, unmatched


def process_rst(path: Path, doi_url_index: dict, author_year_index: dict) -> tuple[str | None, dict, list]:
    original = path.read_text(encoding="utf-8")
    new_content, l2k, unmatched = migrate_doc_text(original, doi_url_index, author_year_index)
    if new_content == original:
        return None, l2k, unmatched
    return new_content, l2k, unmatched


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apply", action="store_true", help="Write changes to disk")
    parser.add_argument("--report", action="store_true", help="Report only, no diffs")
    args = parser.parse_args()

    if not BIB_FILE.exists():
        print(f"ERROR: {BIB_FILE} not found", file=sys.stderr)
        sys.exit(1)

    doi_url_index, author_year_index = load_bib_index(BIB_FILE)
    print(
        f"Loaded {len(doi_url_index)} DOI/URL entries and "
        f"{len(author_year_index)} author+year entries from {BIB_FILE.name}\n"
    )

    all_unmatched: list[tuple[Path, str, str]] = []
    changed: list[Path] = []
    total_migrated = 0
    total_files = 0

    files = [(p, "header") for p in MODEL_HEADERS if p.exists()] + [(p, "rst") for p in STATIC_RST if p.exists()]

    for path, kind in files:
        total_files += 1
        if kind == "header":
            new_content, l2k, unmatched = process_header(path, doi_url_index, author_year_index)
        else:
            new_content, l2k, unmatched = process_rst(path, doi_url_index, author_year_index)

        for label, body in unmatched:
            all_unmatched.append((path, label, body[:80]))

        if new_content is None:
            continue

        total_migrated += len(l2k)
        changed.append(path)

        if not args.report:
            original = path.read_text(encoding="utf-8")
            diff = difflib.unified_diff(
                original.splitlines(keepends=True),
                new_content.splitlines(keepends=True),
                fromfile=str(path.relative_to(REPO)),
                tofile=str(path.relative_to(REPO)) + " (migrated)",
                n=3,
            )
            diff_text = "".join(diff)
            if diff_text:
                print(diff_text)

        if args.apply:
            path.write_text(new_content, encoding="utf-8")
            print(f"  [WROTE] {path.relative_to(REPO)}")

    print(f"\n{'='*60}")
    print(f"Files with changes: {len(changed)} / {total_files}")
    print(f"Total references migrated: {total_migrated}")
    print(f"Unmatched references: {len(all_unmatched)}")
    if all_unmatched:
        print("\nUnmatched references (add to refs.bib or check text format):")
        for path, label, body in all_unmatched:
            print(f"  {path.name}:[{label}] {body}")

    if not args.apply and changed:
        print(f"\nRun with --apply to write {len(changed)} file(s).")


if __name__ == "__main__":
    main()
