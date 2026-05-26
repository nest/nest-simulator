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
    with open(_VERSIONS_PATH) as fp:
        app._nest_versions = json.load(fp)


def version_role(app_ref):
    """Return a Docutils role function that looks up a version from versions.json.

    The role accepts ``package`` or ``package,level`` (default level: ``min``).
    The JSON is read once at builder-init time and cached on the app object.
    """

    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):
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
    app_ref = [app]
    app.connect("builder-inited", lambda _: _load_versions(app_ref[0]))
    app.add_role("version", version_role(app_ref))
    return {"version": "0.2", "parallel_read_safe": True, "parallel_write_safe": True}
