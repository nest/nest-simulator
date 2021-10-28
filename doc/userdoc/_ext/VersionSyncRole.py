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
from docutils import nodes

from conf import doc_build_dir


def version_role(pattern):
    """ Defines the role function

    Args:
        pattern (str): This argument represents the input pattern of the role which is just a string (%s).

    Returns:
        function: Returns the defined role function.
    """

    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):
        """ Actual role implementation.

        Args:
            Refer to https://docutils.sourceforge.io/docs/howto/rst-roles.html for an overview.

        Returns:
            node.Text: Returns the node which contains the version number for a specific key (e.g. package).
        """

        package_level = pattern % text
        payload = package_level.split(',')
        package = payload[0]

        if len(payload) > 1:
            level = payload[1]
        else:
            level = "min"

        with open(str(doc_build_dir) + '/_ext/versions.json') as fp:
            data = json.load(fp)

        try:
            version = data[package.strip()][level.strip()]
        except KeyError as e:
            version = f"'{level}' was not found!"

        node = nodes.Text(version)
        return [node], []
    return role


def setup(app):
    """ Adds the necessary routines to Sphinx.

    Args:
        app (app): Application object.

    Returns:
        dict: returns a dict with version number of the extension and IO safety arguments.
    """

    app.add_role('version', version_role('%s'))

    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
