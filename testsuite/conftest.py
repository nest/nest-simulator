# -*- coding: utf-8 -*-
#
# conftest.py
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
Helper functions available to the entire testsuite directory.
"""

import nest


def get_node_models_by_attribute(attribute: str):
    """Get a list of node model names, such that each model contains the attribute given by ``attribute``."""
    all_models = nest.node_models
    models = [model for model in all_models if attribute in nest.GetDefaults(model)]

    return models
