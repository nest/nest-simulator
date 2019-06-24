# -*- coding: utf-8 -*-
#
# hl_api_spatial.py
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

import numpy as np
from ..lib.hl_api_types import CreateParameter, Parameter
from ..ll_api import sli_func

__all__ = [
    'dimension_distance',
    'distance',
    'grid',
    'free',
    'pos',
    'source_pos',
    'target_pos',
]

distance = CreateParameter('distance', {})


class dimension_distance(object):
    x = CreateParameter('distance', {'dimension': 1})
    y = CreateParameter('distance', {'dimension': 2})
    z = CreateParameter('distance', {'dimension': 3})

    @staticmethod
    def n(dimension):
        return CreateParameter('distance', {'dimension': dimension})


class pos(object):
    x = CreateParameter('position', {'dimension': 0})
    y = CreateParameter('position', {'dimension': 1})
    z = CreateParameter('position', {'dimension': 2})

    @staticmethod
    def n(dimension):
        return CreateParameter('position', {'dimension': dimension})


class source_pos(object):
    x = CreateParameter('position', {'dimension': 0, 'type_id': 1})
    y = CreateParameter('position', {'dimension': 1, 'type_id': 1})
    z = CreateParameter('position', {'dimension': 2, 'type_id': 1})

    @staticmethod
    def n(dimension):
        return CreateParameter('position',
                               {'dimension': dimension, 'type_id': 1})


class target_pos(object):
    x = CreateParameter('position', {'dimension': 0, 'type_id': 2})
    y = CreateParameter('position', {'dimension': 1, 'type_id': 2})
    z = CreateParameter('position', {'dimension': 2, 'type_id': 2})

    @staticmethod
    def n(dimension):
        return CreateParameter('position',
                               {'dimension': dimension, 'type_id': 2})


class grid(object):
    def __init__(self, rows, columns, depth=None, center=None, extent=None,
                 edge_wrap=False):
        self.rows = rows
        self.columns = columns
        self.depth = depth
        self.center = center
        self.extent = extent
        self.edge_wrap = edge_wrap


class free(object):
    def __init__(self, pos, extent=None, edge_wrap=False, num_dimensions=None):
        if extent and num_dimensions:
            raise TypeError(
                'extent and number of dimensions cannot be specified at the' +
                ' same time')
        if isinstance(pos, (list, tuple, np.ndarray)):
            if num_dimensions:
                raise TypeError(
                    'number of dimensions cannot be specified when using an' +
                    ' array of positions')
            self.pos = pos
        elif isinstance(pos, Parameter):
            if extent:
                num_dimensions = len(extent)
            # Number of dimensions is unknown if it cannot be inferred from
            # extent, or if it's not explicitly specified.
            if not num_dimensions:
                raise TypeError(
                    'could not infer number of dimensions. Set ' +
                    'num_dimensions or extent when using Parameter as pos')
            dim_parameters = [pos for _ in range(num_dimensions)]
            if num_dimensions == 2:
                dimfunc = 'dimension2d'
            elif num_dimensions == 3:
                dimfunc = 'dimension3d'
            else:
                raise ValueError('Number of dimensions must be 2 or 3.')
            # The dimension2d and dimension3d Parameter stores a Parameter for
            # each dimension. When creating positions for nodes, values from
            # each parameter are fetched for the position vector.
            self.pos = sli_func(dimfunc, *dim_parameters)
        else:
            raise TypeError(
                'pos must be either an array of positions, or a Parameter')

        self.extent = extent
        self.edge_wrap = edge_wrap
