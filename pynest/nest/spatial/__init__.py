# -*- coding: utf-8 -*-
#
# __init__.py
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

import functools as _functools
from .hl_api_spatial import *  # noqa
from .hl_api_spatial import DistanceParameter as _DistanceParameter


@_functools.cache
def __getattr__(name):
    if name == "distance":
        return _DistanceParameter()
    raise AttributeError(f"module {__name__} has no attribute {name}")


# Type annotation to hint at dynamic singleton of DistanceParameter()
distance: _DistanceParameter
"""
A singleton instance representing the distance between two nodes in space.

If used alone, the DistanceObject represents simply the Euclidean
distance between two nodes.

Alternatively the distance in a single dimension may be chosen. Three
properties are defined, x, y, and z, which represent the distance in
their respective dimensions. Note that the distance parameter can only
be used in contexts with two nodes, e.g. when connecting.
"""


__all__ = [attr for attr in vars().keys() if not attr.startswith("_")]
