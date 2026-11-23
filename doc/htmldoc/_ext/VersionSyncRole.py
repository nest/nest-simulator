# -*- coding: utf-8 -*-
#
# VersionSyncRole.py
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

import json
from pathlib import Path

from docutils import nodes
from sphinx.util import logging

logger = logging.getLogger(__name__)
_VERSIONS_PATH = Path(__file__).parent / "versions.json"


def _load_versions(app):
    """Read versions.json and cache its contents on the Sphinx app.

    Called once when the builder is initialized so that the file is read a
    single time per build rather than on every use of the role.

    Args:
        app (sphinx.application.Sphinx): The Sphinx application object. The
            parsed JSON is stored on it as ``app._nest_versions``.
    """
    with open(_VERSIONS_PATH) as fp:
        app._nest_versions = json.load(fp)


def version_role(app_ref):
    """Return a Docutils role function that looks up a version from versions.json.

    The role accepts ``package`` or ``package,level`` (default level: ``min``).
    The JSON is read once at builder-init time and cached on the app object.

    Args:
        app_ref (list): A single-element list holding the Sphinx application
            object. A list is used so the closure can reach the app instance
            once it has been populated by :func:`setup`.

    Returns:
        function: The Docutils role function.
    """

    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):
        """Resolve a version number for a ``:version:`` role.

        See https://docutils.sourceforge.io/docs/howto/rst-roles.html for an
        overview of the role function signature. The role's ``text`` is either
        a package name or ``package,level``; ``level`` defaults to ``min``. If
        the requested key is missing, a build warning is emitted and a visible
        placeholder is inserted instead.

        Returns:
            tuple: ``([nodes.Text], [])`` containing the resolved version
            number (or a placeholder) and an empty list of system messages.
        """
        data = app_ref[0]._nest_versions
        parts = text.split(",", 1)
        package = parts[0].strip()
        level = parts[1].strip() if len(parts) > 1 else "min"
        try:
            version = data[package][level]
        except KeyError:
            logger.warning(
                f"versions.json: key '{package}.{level}' not found (used at line {lineno})",
                type="versionsync",
            )
            version = f"[{package}.{level}?]"
        return [nodes.Text(version)], []

    return role


def setup(app):
    """Register the extension with Sphinx.

    Connects :func:`_load_versions` to the ``builder-inited`` event so the
    version data is loaded once per build, and adds the ``version`` role.

    Args:
        app (sphinx.application.Sphinx): The Sphinx application object.

    Returns:
        dict: Extension metadata with its version number and the parallel
        read/write safety flags.
    """
    app_ref = [app]
    app.connect("builder-inited", lambda _: _load_versions(app_ref[0]))
    app.add_role("version", version_role(app_ref))
    return {"version": "0.2", "parallel_read_safe": True, "parallel_write_safe": True}
