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
    'distance',
    'grid',
    'free',
    'pos',
    'source_pos',
    'target_pos',
]


class DistanceParameter(Parameter):
    """
    Object representing the distance between two nodes in space.

    If used alone, the DistanceObject represents simply the Euclidean
    distance between two nodes.

    Alternatively the distance in a single dimension may be chosen. Three
    properties are defined, x, y, and z, which represent the distance in
    their respective dimensions. Note that the distance parameter can only
    be used in contexts with two nodes, e.g. when connecting.
    """

    def __init__(self):
        distance_parameter = CreateParameter('distance', {})
        super().__init__(distance_parameter._datum)

    @property
    def x(self):
        """Parameter representing the distance on the x-axis"""
        return CreateParameter('distance', {'dimension': 1})

    @property
    def y(self):
        """Parameter representing the distance on the y-axis"""
        return CreateParameter('distance', {'dimension': 2})

    @property
    def z(self):
        """Parameter representing the distance on the z-axis"""
        return CreateParameter('distance', {'dimension': 3})

    @staticmethod
    def n(dimension):
        """
        Distance in given dimension.

        Parameters
        ----------
        dimension : int
            Dimension in which to get the distance.

        Returns
        -------
        Parameter:
            Object yielding the distance in the given dimension.
        """
        return CreateParameter('distance', {'dimension': dimension})


distance = DistanceParameter()


class pos(object):
    """
    Position of node in a specific dimension.

    Three properties are defined, x, y, and z, which represent the
    position in their respective dimensions. Note that this parameter can
    only be used in contexts with one node, e.g. when setting node status.
    """
    x = CreateParameter('position', {'dimension': 0})
    y = CreateParameter('position', {'dimension': 1})
    z = CreateParameter('position', {'dimension': 2})

    @staticmethod
    def n(dimension):
        """
        Position in given dimension.

        Parameters
        ----------
        dimension : int
            Dimension in which to get the position.

        Returns
        -------
        Parameter:
            Object yielding the position in the given dimension.
        """
        return CreateParameter('position', {'dimension': dimension})


class source_pos(object):
    """
    Position of the source node in a specific dimension.

    Three properties are defined, x, y, and z, which represent the source
    node position in their respective dimensions. Note that this parameter
    can only be used in contexts with two nodes, e.g. when connecting.
    """
    x = CreateParameter('position', {'dimension': 0, 'synaptic_endpoint': 1})
    y = CreateParameter('position', {'dimension': 1, 'synaptic_endpoint': 1})
    z = CreateParameter('position', {'dimension': 2, 'synaptic_endpoint': 1})

    @staticmethod
    def n(dimension):
        """
        Position of source node in given dimension.

        Parameters
        ----------
        dimension : int
            Dimension in which to get the position.

        Returns
        -------
        Parameter:
            Object yielding the position in the given dimension.
        """
        return CreateParameter('position',
                               {'dimension': dimension, 'synaptic_endpoint': 1})


class target_pos(object):
    """
    Position of the target node in a specific dimension.

    Three properties are defined, x, y, and z, which represent the target
    node position in their respective dimensions. Note that this parameter
    can only be used in contexts with two nodes, e.g. when connecting.
    """
    x = CreateParameter('position', {'dimension': 0, 'synaptic_endpoint': 2})
    y = CreateParameter('position', {'dimension': 1, 'synaptic_endpoint': 2})
    z = CreateParameter('position', {'dimension': 2, 'synaptic_endpoint': 2})

    @staticmethod
    def n(dimension):
        """
        Position of target node in given dimension.

        Parameters
        ----------
        dimension : int
            Dimension in which to get the position.

        Returns
        -------
        Parameter:
            Object yielding the position in the given dimension.
        """
        return CreateParameter('position',
                               {'dimension': dimension, 'synaptic_endpoint': 2})


class grid(object):
    """
    Defines grid-based positions for nodes.

    Parameters
    ----------
    shape : list
        Two- or three-element list with the grid shape in two or three dimensions, respectively.
    center : list, optional
        Position of the center of the layer.
    extent : list, optional
        Extent of the layer in each dimension.
    edge_wrap : bool, optional
        Specifies periodic boundary conditions.
    """

    def __init__(self, shape, center=None, extent=None, edge_wrap=False):
        self.shape = shape
        self.center = center
        self.extent = extent
        self.edge_wrap = edge_wrap


class free(object):
    """
    Defines positions for nodes based on a list of positions, or a Parameter object.

    Parameters
    ----------
    pos : [list | Parameter]
        Either a list of two- or three-element lists containing positions, depending on number of dimensions,
        a two- or three-element list of Parameters, depending on number of dimensions,
        or a single Parameter.
    extent : list, optional
        Extent of the layer in each dimension.
    edge_wrap : bool, optional
        Specifies periodic boundary conditions.
    num_dimensions : int, optional
        If a single Parameter is given as position, and no extent is
        specified, the number of dimensions must be set explicitly.
    """

    def __init__(self, pos, extent=None, edge_wrap=False, num_dimensions=None):
        if extent and num_dimensions:
            raise TypeError(
                'extent and number of dimensions cannot be specified at the'
                ' same time')
        if isinstance(pos, (list, tuple, np.ndarray)):
            if num_dimensions:
                raise TypeError(
                    'number of dimensions cannot be specified when using an'
                    ' array of positions')
            if len(pos) == sum(isinstance(d, Parameter) for d in pos):
                self.pos = self._parameter_list_to_dimension(pos, len(pos))
            else:
                self.pos = pos
        elif isinstance(pos, Parameter):
            if extent:
                num_dimensions = len(extent)
            # Number of dimensions is unknown if it cannot be inferred from
            # extent, or if it's not explicitly specified.
            if not num_dimensions:
                raise TypeError(
                    'could not infer number of dimensions. Set '
                    'num_dimensions or extent when using Parameter as pos')
            dim_parameters = [pos for _ in range(num_dimensions)]
            self.pos = self._parameter_list_to_dimension(dim_parameters, num_dimensions)
        else:
            raise TypeError(
                'pos must be either an array of positions, or a Parameter')

        self.extent = extent
        self.edge_wrap = edge_wrap

    def _parameter_list_to_dimension(self, dim_parameters, num_dimensions):
        """Converts a list of Parameters to a dimension2d or dimension3d Parameter."""
        if num_dimensions == 2:
            dimfunc = 'dimension2d'
        elif num_dimensions == 3:
            dimfunc = 'dimension3d'
        else:
            raise ValueError('Number of dimensions must be 2 or 3.')
        # The dimension2d and dimension3d Parameter stores a Parameter for
        # each dimension. When creating positions for nodes, values from
        # each parameter are fetched for the position vector.
        return sli_func(dimfunc, *dim_parameters)
