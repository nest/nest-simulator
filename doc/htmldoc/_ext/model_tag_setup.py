# -*- coding: utf-8 -*-
#
# model_tag_setup.py
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

"""Render NEST model user documentation into reStructuredText.

Model documentation is written inside ``BeginUserDocs``/``EndUserDocs`` blocks in the
C++ headers under ``models/`` and ``nestkernel/``. This extension

* extracts each block and writes it as a standalone page under ``doc/htmldoc/models/``,
* collects the block tags so ``models/index`` can offer a tag filter, and
* renders the pages that use those tags as Jinja templates.

Every block begins with a ``Short description`` section whose single paragraph names the
model. That paragraph becomes the page title; the remainder of the block is copied
through unchanged.
"""

from __future__ import annotations

import json
import re
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path
from typing import Any, Sequence

from sphinx.util import logging

logger = logging.getLogger(__name__)

#: Section whose contents become the title of a generated page.
SHORT_DESCRIPTION = "Short description"

#: Separates the model name from its short description in a page title.
EN_DASH = "–"

#: Characters reStructuredText accepts as section adornment.
ADORNMENTS = frozenset("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~")

#: Directories, relative to the repository root, that are searched for headers.
HEADER_DIRS = ("models", "nestkernel")

#: Tag that keeps a model out of the tag filter but leaves it in the toctree.
NOINDEX = "NOINDEX"

#: Pages rendered as Jinja templates against the collected model and tag data.
TEMPLATE_PAGES = frozenset({"models/index", "neurons/index", "synapses/index", "devices/index", "neurons/neuron_types"})

#: A ``BeginUserDocs`` block: a tag line, a blank line, then the documentation itself.
USERDOC_RE = re.compile(r"BeginUserDocs:[ \t]*(?P<tags>.*?)\n\n(?P<doc>.*?)(?=EndUserDocs)", re.DOTALL)


class UserDocError(ValueError):
    """Raised when a user documentation block is not shaped as expected."""


@dataclass(frozen=True)
class UserDoc:
    """One ``BeginUserDocs`` block extracted from a C++ header."""

    path: Path
    tags: tuple[str, ...]
    body: str

    @property
    def stem(self) -> str:
        """Model name, taken from the header file name."""
        return self.path.stem


# The following functions turn a documentation block into a reST page.


def _skip_blank(lines: Sequence[str], index: int) -> int:
    """Return the index of the first non-blank line at or after `index`."""
    while index < len(lines) and not lines[index].strip():
        index += 1
    return index


def _take_paragraph(lines: Sequence[str], index: int) -> tuple[list[str], int]:
    """Collect stripped lines up to the next blank one, with the index that follows."""
    paragraph = []
    while index < len(lines) and lines[index].strip():
        paragraph.append(lines[index].strip())
        index += 1
    return paragraph, index


def render_userdoc(body: str, stem: str, *, heading: str = SHORT_DESCRIPTION) -> str:
    """Render one extracted documentation block as a standalone reST document.

    The block's `heading` section holds a single paragraph naming the model, which
    becomes the document title. Everything after that paragraph is copied through
    verbatim. Since the paragraph ends at the first blank line, section markup further
    down cannot be drawn into the title, whatever adornment characters it uses.

    Parameters
    ----------
    body : str
        Documentation block as extracted from the header.
    stem : str
        Model name, used as the first half of the title.
    heading : str
        Title of the section that supplies the short description.

    Returns
    -------
    str
        reStructuredText with the `heading` section replaced by a document title.

    Raises
    ------
    UserDocError
        If the block does not begin with a non-empty, underlined `heading` section.
    """
    lines = body.split("\n")
    found = lines[0].strip() if lines else ""
    if len(lines) < 2 or found != heading:
        raise UserDocError(f"user documentation must start with a {heading!r} section, found {found!r}")

    underline = lines[1].rstrip()
    if len(underline) < 3 or len(set(underline)) != 1 or underline[0] not in ADORNMENTS:
        raise UserDocError(f"{heading!r} is not underlined by a section adornment, found {underline!r}")

    paragraph, index = _take_paragraph(lines, _skip_blank(lines, 2))
    if not paragraph:
        raise UserDocError(f"the {heading!r} section is empty")

    title = f"{stem} {EN_DASH} {' '.join(paragraph)}"
    return "\n".join([title, "=" * len(title), "", *lines[_skip_blank(lines, index) :]])


# The following functions read the headers. The result is cached because both the
# config-inited and the env-before-read-docs handler need it.


@lru_cache(maxsize=None)
def _collect_userdocs(srcdir: str) -> tuple[UserDoc, ...]:
    """Read every header below the repository root once and return its documentation.

    Parameters
    ----------
    srcdir : str
        Sphinx source directory, two levels below the repository root.

    Returns
    -------
    tuple of UserDoc
        One entry per header that carries a documentation block, sorted by file name so
        that the generated toctree and tag data are reproducible.
    """
    root = Path(srcdir).parents[1]
    userdocs = []
    for directory in HEADER_DIRS:
        for path in sorted((root / directory).glob("*.h")):
            match = USERDOC_RE.search(path.read_text(encoding="utf-8"))
            if match is None:
                logger.debug("no user documentation in %s", path)
                continue
            tags = tuple(tag.strip() for tag in match["tags"].split(",") if tag.strip())
            userdocs.append(UserDoc(path=path, tags=tags, body=match["doc"]))
    return tuple(userdocs)


# The following function is called at Sphinx core event config-inited.


def create_rst_files(app: Any, config: Any) -> None:
    """Write one reST page per documented model into the ``models`` source directory.

    A block that cannot be rendered is reported as a build warning and written with the
    model name as its title, so that the page stays valid for the toctree and the
    problem is visible rather than silent.

    Parameters
    ----------
    app
        Sphinx application object.
    config
        Sphinx config object; unused, required by the event signature.
    """
    outdir = Path(app.srcdir) / "models"
    outdir.mkdir(parents=True, exist_ok=True)
    for userdoc in _collect_userdocs(str(app.srcdir)):
        try:
            text = render_userdoc(userdoc.body, userdoc.stem)
        except UserDocError as error:
            logger.warning("%s", error, location=str(userdoc.path))
            text = "\n".join([userdoc.stem, "=" * len(userdoc.stem), "", userdoc.body])
        (outdir / f"{userdoc.stem}.rst").write_text(text, encoding="utf-8")


# The following functions are called at Sphinx core event env-before-read-docs.


def get_model_tags(app: Any, env: Any, docname: str) -> None:
    """Store model and tag data on the build environment and export it as JSON.

    Parameters
    ----------
    app
        Sphinx application object.
    env
        Sphinx build environment, which carries the data to the template renderer.
    docname : str
        Name of the document being processed; unused, required by the event signature.

    Note
    ----
    Writes ``static/data/filter_model.json``, which is loaded client-side.
    """
    env.model_dict = prepare_model_dict(app)
    env.tag_dict = find_models_in_tag_combinations(env.model_dict)

    json_output = Path(app.srcdir, "static", "data", "filter_model.json")
    json_output.parent.mkdir(parents=True, exist_ok=True)
    json_output.write_text(json.dumps(env.tag_dict, indent=2), encoding="utf-8")


def prepare_model_dict(app: Any) -> dict[str, list[str]]:
    """Map each generated page to the tags of its documentation block.

    A model tagged ``NOINDEX`` keeps its entry, but with no tags: it stays in the
    toctree while staying out of the tag filter.

    Parameters
    ----------
    app
        Sphinx application object.

    Returns
    -------
    dict
        Page file names mapped to their list of tags.

    Example
    -------
    A header ``example.h`` beginning with ``BeginUserDocs: neuron, integrate-and-fire``
    yields ``{"example.html": ["neuron", "integrate-and-fire"]}``.
    """
    return {
        f"{userdoc.stem}.html": [] if NOINDEX in userdoc.tags else list(userdoc.tags)
        for userdoc in _collect_userdocs(str(app.srcdir))
    }


def find_models_in_tag_combinations(models_dict: dict[str, list[str]]) -> list[dict[str, Any]]:
    """Invert a model-to-tags mapping into a list of tags with their models.

    Parameters
    ----------
    models_dict : dict
        Model identifiers mapped to their lists of tags.

    Returns
    -------
    list of dict
        One entry per tag, holding the tag, its models, and how many there are.
    """
    model_to_tags: dict[str, set[str]] = {}
    for model, tags in models_dict.items():
        for tag in tags:
            model_to_tags.setdefault(tag, set()).add(model)

    return [
        {"tag": tag, "models": sorted(models), "count": len(models)} for tag, models in sorted(model_to_tags.items())
    ]


# The following function is called at Sphinx core event source-read.


def template_renderer(app: Any, docname: str, source: list[str]) -> None:
    """Render the pages that present model and tag data as Jinja templates.

    Parameters
    ----------
    app
        Sphinx application object.
    docname : str
        Name of the document being processed.
    source : list
        Single-element list holding the document source, modified in place.
    """
    if docname not in TEMPLATE_PAGES:
        return

    env = app.builder.env
    html_context = {"tag_dict": env.tag_dict, "model_dict": env.model_dict}
    source[0] = app.builder.templates.render_string(source[0], html_context)


def setup(app: Any) -> dict[str, Any]:
    """Connect the handlers above to the Sphinx events that drive them."""
    app.connect("config-inited", create_rst_files)
    app.connect("env-before-read-docs", get_model_tags)
    app.connect("source-read", template_renderer)

    return {
        "version": "0.2",
        "env_version": 1,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
