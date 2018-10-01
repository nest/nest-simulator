# -*- coding: utf-8 -*-
#
# hl_api.py
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
**High-level API of PyNEST Topology Module**

This file defines the user-level functions of NEST's Python interface to the
Topology module. The basic approach is the same as for the PyNEST interface to
NEST:

1.  Function names are the same as in SLI.
2.  Nodes are identified by their GIDs.
3.  GIDs are always given as tuples or lists of integer(s).
4.  Commands returning GIDs return them as tuples.
5.  Other arguments can be

    * single items that are applied to all entries in a GID list
    * a list of the same length as the given list of GID(s) where each item is
      matched with the pertaining GID.

    **Example**
        ::

            layers = CreateLayer(({...}, {...}, {...}))

    creates three layers and returns a tuple of three GIDs.
        ::

            ConnectLayers(layers[:2], layers[1:], {...})

    connects `layers[0]` to `layers[1]` and `layers[1]` to `layers[2]` \
using the same dictionary to specify both connections.
        ::

            ConnectLayers(layers[:2], layers[1:], ({...}, {...}))

    connects the same layers, but the `layers[0]` to `layers[1]` connection
    is specified by the first dictionary, the `layers[1]` to `layers[2]`
    connection by the second.


:Authors:
    Kittel Austvoll,
    Hans Ekkehard Plesser,
    Hakon Enger
"""

import nest
import nest.lib.hl_api_helper as hlh


def topology_func(slifunc, *args):
    """
    Execute SLI function `slifunc` with arguments `args` in Topology namespace.


    Parameters
    ----------
    slifunc : str
        SLI namespace expression


    Other parameters
    ----------------
    args : dict
        An arbitrary number of arguments


    Returns
    -------
    out :
        Values from SLI function `slifunc`


    See also
    --------
    nest.sli_func
    """

    return nest.sli_func(slifunc, *args)


class Mask(object):
    """
    Class for spatial masks.

    Masks are used when creating connections in the Topology module. A mask
    describes which area of the pool layer shall be searched for nodes to
    connect for any given node in the driver layer. Masks are created using
    the ``CreateMask`` command.
    """

    _datum = None

    # The constructor should not be called by the user
    def __init__(self, datum):
        """Masks must be created using the CreateMask command."""
        if not isinstance(datum, nest.SLIDatum) or datum.dtype != "masktype":
            raise TypeError("expected mask Datum")
        self._datum = datum

    # Generic binary operation
    def _binop(self, op, other):
        if not isinstance(other, Mask):
            return NotImplemented
        return Mask(topology_func(op, self._datum, other._datum))

    def __or__(self, other):
        return self._binop("or", other)

    def __and__(self, other):
        return self._binop("and", other)

    def __sub__(self, other):
        return self._binop("sub", other)

    def Inside(self, point):
        """
        Test if a point is inside a mask.


        Parameters
        ----------
        point : tuple/list of float values
            Coordinate of point


        Returns
        -------
        out : bool
            True if the point is inside the mask, False otherwise
        """
        return topology_func("Inside", point, self._datum)


def CreateMask(masktype, specs, anchor=None):
    """
    Create a spatial mask for connections.

    Masks are used when creating connections in the Topology module. A mask
    describes the area of the pool layer that is searched for nodes to
    connect for any given node in the driver layer. Several mask types
    are available. Examples are the grid region, the rectangular, circular or
    doughnut region.

    The command ``CreateMask`` creates a Mask object which may be combined
    with other ``Mask`` objects using Boolean operators. The mask is specified
    in a dictionary.

    ``Mask`` objects can be passed to ``ConnectLayers`` in a
    connection dictionary with the key `'mask'`.


    Parameters
    ----------
    masktype : str, ['rectangular' | 'circular' | 'doughnut' | 'elliptical']
        for 2D masks, \ ['box' | 'spherical' | 'ellipsoidal] for 3D masks,
        ['grid'] only for grid-based layers in 2D
        The mask name corresponds to the geometrical shape of the mask. There
        are different types for 2- and 3-dimensional layers.
    specs : dict
        Dictionary specifying the parameters of the provided `masktype`,
        see **Notes**.
    anchor : [tuple/list of floats | dict with the keys `'column'` and \
`'row'` (for grid masks only)], optional, default: None
        By providing anchor coordinates, the location of the mask relative to
        the driver node can be changed. The list of coordinates has a length
        of 2 or 3 dependent on the number of dimensions.


    Returns
    -------
    out : ``Mask`` object


    See also
    --------
    ConnectLayers: Connect two (lists of) layers pairwise according to
        specified projections. ``Mask`` objects can be passed in a connection
        dictionary with the key `'mask'`.


    Notes
    -----
    - All angles must be given in degrees.


    **Mask types**

    Available mask types (`masktype`) and their corresponding parameter
    dictionaries:

    * 2D free and grid-based layers
        ::

            'rectangular' :
                {'lower_left'   : [float, float],
                 'upper_right'  : [float, float],
                 'azimuth_angle': float  # default:0.0}
            #or
            'circular' :
                {'radius' : float}
            #or
            'doughnut' :
                {'inner_radius' : float,
                 'outer_radius' : float}
            #or
            'elliptical' :
                {'major_axis' : float,
                 'minor_axis' : float,
                 'azimuth_angle' : float,   # default: 0.0,
                 'anchor' : [float, float], # default: [0.0, 0.0]}


    * 3D free and grid-based layers
        ::

            'box' :
                {'lower_left'  : [float, float, float],
                 'upper_right' : [float, float, float],
                 'azimuth_angle: float  # default: 0.0,
                 'polar_angle  : float  # defualt: 0.0}
            #or
            'spherical' :
                {'radius' : float}
            #or
            'ellipsoidal' :
                {'major_axis' : float,
                 'minor_axis' : float,
                 'polar_axis' : float
                 'azimuth_angle' : float,   # default: 0.0,
                 'polar_angle' : float,     # default: 0.0,
                 'anchor' : [float, float, float], # default: [0.0, 0.0, 0.0]}}


    * 2D grid-based layers only
        ::

            'grid' :
                {'rows' : float,
                 'columns' : float}

        By default the top-left corner of a grid mask, i.e., the grid
        mask element with grid index [0, 0], is aligned with the driver
        node. It can be changed by means of the 'anchor' parameter:
            ::

                'anchor' :
                    {'row' : float,
                     'column' : float}


    **Example**
        ::

            import nest.topology as tp

            # create a grid-based layer
            l = tp.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # create a circular mask
            m = tp.CreateMask('circular', {'radius': 0.2})

            # connectivity specifications
            conndict = {'connection_type': 'divergent',
                        'mask'           : m}

            # connect layer l with itself according to the specifications
            tp.ConnectLayers(l, l, conndict)

    """

    if anchor is None:
        return Mask(topology_func('CreateMask', {masktype: specs}))
    else:
        return Mask(
            topology_func('CreateMask', {masktype: specs, 'anchor': anchor}))


class Parameter(object):
    """
    Class for parameters for distance dependency or randomization.

    Parameters are spatial functions which are used when creating
    connections in the Topology module. A parameter may be used as a
    probability kernel when creating connections or as synaptic parameters
    (such as weight and delay). Parameters are created using the
    ``CreateParameter`` command.
    """

    _datum = None

    # The constructor should not be called by the user
    def __init__(self, datum):
        """Parameters must be created using the CreateParameter command."""
        if not isinstance(datum,
                          nest.SLIDatum) or datum.dtype != "parametertype":
            raise TypeError("expected parameter datum")
        self._datum = datum

    # Generic binary operation
    def _binop(self, op, other):
        if not isinstance(other, Parameter):
            return NotImplemented
        return Parameter(topology_func(op, self._datum, other._datum))

    def __add__(self, other):
        return self._binop("add", other)

    def __sub__(self, other):
        return self._binop("sub", other)

    def __mul__(self, other):
        return self._binop("mul", other)

    def __div__(self, other):
        return self._binop("div", other)

    def __truediv__(self, other):
        return self._binop("div", other)

    def GetValue(self, point):
        """
        Compute value of parameter at a point.


        Parameters
        ----------
        point : tuple/list of float values
            coordinate of point


        Returns
        -------
        out : value
            The value of the parameter at the point


        See also
        --------
        CreateParameter : create parameter for e.g., distance dependency


        Notes
        -----
        -


        **Example**
            ::

                import nest.topology as tp

                #linear dependent parameter
                P = tp.CreateParameter('linear', {'a' : 2., 'c' : 0.})

                #get out value
                P.GetValue(point=[3., 4.])

        """
        return topology_func("GetValue", point, self._datum)


def CreateParameter(parametertype, specs):
    """
    Create a parameter for distance dependency or randomization.

    Parameters are (spatial) functions which are used when creating
    connections in the Topology module for distance dependency or
    randomization. This command creates a Parameter object which may be
    combined with other ``Parameter`` objects using arithmetic operators.
    The parameter is specified in a dictionary.

    A parameter may be used as a probability kernel when creating connections
    or as synaptic parameters (such as weight and delay), i.e., for specifying
    the parameters `'kernel'`, `'weights'` and `'delays'` in the
    connection dictionary passed to ``ConnectLayers``.


    Parameters
    ----------
    parametertype : {'constant', 'linear', 'exponential', 'gaussian', \
'gaussian2D', 'uniform', 'normal', 'lognormal'}
        Function types with or without distance dependency
    specs : dict
        Dictionary specifying the parameters of the provided
        `'parametertype'`, see **Notes**.


    Returns
    -------
    out : ``Parameter`` object


    See also
    --------
    ConnectLayers : Connect two (lists of) layers pairwise according to
        specified projections. Parameters can be used to specify the
        parameters `'kernel'`, `'weights'` and `'delays'` in the
        connection dictionary.
    Parameters : Class for parameters for distance dependency or randomization.


    Notes
    -----
    -


    **Parameter types**

    Available parameter types (`parametertype` parameter), their function and
    acceptable keys for their corresponding specification dictionaries

    * Constant
        ::

            'constant' :
                {'value' : float} # constant value

    * With dependence on the distance `d`
        ::

            # p(d) = c + a * d
            'linear' :
                {'a' : float, # slope, default: 1.0
                 'c' : float} # constant offset, default: 0.0
            # or
            # p(d) = c + a*exp(-d/tau)
            'exponential' :
                {'a'   : float, # coefficient of exponential term, default: 1.0
                 'c'   : float, # constant offset, default: 0.0
                 'tau' : float} # length scale factor, default: 1.0
            # or
            # p(d) = c + p_center*exp(-(d-mean)^2/(2*sigma^2))
            'gaussian' :
                {'p_center' : float, # value at center, default: 1.0
                 'mean'     : float, # distance to center, default: 0.0
                 'sigma'    : float, # width of Gaussian, default: 1.0
                 'c'        : float} # constant offset, default: 0.0

    * Bivariate Gaussian parameter:
        ::

            # p(x,y) = c + p_center *
            #          exp( -( (x-mean_x)^2/sigma_x^2 + (y-mean_y)^2/sigma_y^2
            #          + 2*rho*(x-mean_x)*(y-mean_y)/(sigma_x*sigma_y) ) /
            #          (2*(1-rho^2)) )
            'gaussian2D' :
                {'p_center' : float, # value at center, default: 1.0
                 'mean_x'   : float, # x-coordinate of center, default: 0.0
                 'mean_y'   : float, # y-coordinate of center, default: 0.0
                 'sigma_x'  : float, # width in x-direction, default: 1.0
                 'sigma_y'  : float, # width in y-direction, default: 1.0
                 'rho'      : float, # correlation of x and y, default: 0.0
                 'c'        : float} # constant offset, default: 0.0

    * Without distance dependency, for randomization
        ::

            # random parameter with uniform distribution in [min,max)
            'uniform' :
                {'min' : float, # minimum value, default: 0.0
                 'max' : float} # maximum value, default: 1.0
            # or
            # random parameter with normal distribution, optionally truncated
            # to [min,max)
            'normal':
                {'mean' : float, # mean value, default: 0.0
                 'sigma': float, # standard deviation, default: 1.0
                 'min'  : float, # minimum value, default: -inf

                 'max'  : float} # maximum value, default: +inf
            # or
            # random parameter with lognormal distribution,
            # optionally truncated to [min,max)
            'lognormal' :
                {'mu'   : float, # mean value of logarithm, default: 0.0
                 'sigma': float, # standard deviation of log, default: 1.0
                 'min'  : float, # minimum value, default: -inf
                 'max'  : float} # maximum value, default: +inf


    **Example**
        ::

            import nest.topology as tp

            # create a grid-based layer
            l = tp.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # parameter for delay with linear distance dependency
            d = tp.CreateParameter('linear', {'a': 0.2,
                                              'c': 0.2})

            # connectivity specifications
            conndict = {'connection_type': 'divergent',
                        'delays': d}

            tp.ConnectLayers(l, l, conndict)

    """
    return Parameter(topology_func('CreateParameter', {parametertype: specs}))


def CreateLayer(specs):
    """
    Create one ore more Topology layer(s) according to given specifications.

    The Topology module organizes neuronal networks in layers. A layer is a
    special type of subnet which contains information about the spatial
    position of its nodes (simple or composite elements) in 2 or 3 dimensions.

    If `specs` is a dictionary, a single layer is created. If it is a list
    of dictionaries, one layer is created for each dictionary.

    Topology distinguishes between two classes of layers:

        * grid-based layers in which each element is placed at a location in a
          regular grid
        * free layers in which elements can be placed arbitrarily

    Obligatory dictionary entries define the class of layer
    (grid-based layers: 'columns' and 'rows'; free layers: 'positions')
    and the 'elements'.


    Parameters
    ----------
    specs : (tuple/list of) dict(s)
        Dictionary or list of dictionaries with layer specifications, see
        **Notes**.

    Returns
    -------
    out : tuple of int(s)
        GID(s) of created layer(s)


    See also
    --------
    ConnectLayers: Connect two (lists of) layers which were created with
        ``CreateLayer`` pairwise according to specified projections.


    Other parameters
    ----------------
    Available parameters for the layer-specifying dictionary `specs`
    center : tuple/list of floats, optional, default: (0.0, 0.0)
        Layers are centered about the origin by default, but the center
        coordinates can also be changed.
        'center' has length 2 or 3 dependent on the number of dimensions.
    columns : int, obligatory for grid-based layers
        Number of columns.
        Needs `'rows'`; mutually exclusive with `'positions'`.
    edge_wrap : bool, default: False
        Periodic boundary conditions.
    elements : (tuple/list of) str or str followed by int
        Elements of layers are NEST network nodes such as neuron models or
        devices.
        For network elements with several nodes of the same type, the
        number of nodes to be created must follow the model name.
        For composite elements, a collection of nodes can be passed as
        list or tuple.
    extent : tuple of floats, optional, default in 2D: (1.0, 1.0)
        Size of the layer. It has length 2 or 3 dependent on the number of
        dimensions.
    positions : tuple/list of coordinates (lists/tuples of floats),
        obligatory for free layers
        Explicit specification of the positions of all elements.
        The coordinates have a length 2 or 3 dependent on the number of
        dimensions.
        All element positions must be within the layer's extent.
        Mutually exclusive with 'rows' and 'columns'.
    rows : int, obligatory for grid-based layers
        Number of rows.
        Needs `'columns'`; mutually exclusive with `'positions'`.


    Notes
    -----
    -


    **Example**
        ::

            import nest
            import nest.topology as tp

            # grid-based layer
            gl = tp.CreateLayer({'rows'      : 5,
                                 'columns'   : 5,
                                 'elements'  : 'iaf_psc_alpha'})

            # free layer
            import numpy as np
            pos = [[np.random.uniform(-0.5, 0.5), np.random.uniform(-0.5,0.5)]
                    for i in range(50)]
            fl = tp.CreateLayer({'positions' : pos,
                                 'elements'  : 'iaf_psc_alpha'})

            # extent, center and edge_wrap
            el = tp.CreateLayer({'rows'      : 5,
                                 'columns'   : 5,
                                 'extent'    : [2.0, 3.0],
                                 'center'    : [1.0, 1.5],
                                 'edge_wrap' : True,
                                 'elements'  : 'iaf_psc_alpha'})

            # composite layer with several nodes of the same type
            cl = tp.CreateLayer({'rows'      : 1,
                                 'columns'   : 2,
                                 'elements'  : ['iaf_cond_alpha', 10,
                                               'poisson_generator',
                                               'noise_generator', 2]})

            # investigate the status dictionary of a layer
            nest.GetStatus(gl)[0]['topology']

    """

    if isinstance(specs, dict):
        specs = (specs, )
    elif not all(isinstance(spec, dict) for spec in specs):
        raise TypeError("specs must be a dictionary or a list of dictionaries")

    for dicts in specs:
        elements = dicts['elements']
        if isinstance(elements, list):
            for elem in elements:
                hlh.model_deprecation_warning(elem)
        else:
            hlh.model_deprecation_warning(elements)

    return topology_func('{ CreateLayer } Map', specs)


def ConnectLayers(pre, post, projections):
    """
    Pairwise connect of pre- and postsynaptic (lists of) layers.

    `pre` and `post` must be a tuple/list of GIDs of equal length. The GIDs
    must refer to layers created with ``CreateLayers``. Layers in the `pre`
    and `post` lists are connected pairwise.

    * If `projections` is a single dictionary, it applies to all pre-post
      pairs.
    * If `projections` is a tuple/list of dictionaries, it must have the same
      length as `pre` and `post` and each dictionary is matched with the proper
      pre-post pair.

    A minimal call of ``ConnectLayers`` expects a source layer `pre`, a
    target layer `post` and a connection dictionary `projections`
    containing at least the entry `'connection_type'` (either
    `'convergent'` or `'divergent'`).

    When connecting two layers, the driver layer is the one in which each node
    is considered in turn. The pool layer is the one from which nodes are
    chosen for each node in the driver layer.


    Parameters
    ----------
    pre : tuple/list of int(s)
        List of GIDs of presynaptic layers (sources)
    post : tuple/list of int(s)
        List of GIDs of postsynaptic layers (targets)
    projections : (tuple/list of) dict(s)
        Dictionary or list of dictionaries specifying projection properties


    Returns
    -------
    out : None
        ConnectLayers returns `None`


    See also
    --------
    CreateLayer : Create one or more Topology layer(s).
    CreateMask : Create a ``Mask`` object. Documentation on available spatial
        masks. Masks can be used to specify the key `'mask'` of the
        connection dictionary.
    CreateParameter : Create a ``Parameter`` object. Documentation on available
        parameters for distance dependency and randomization. Parameters can
        be used to specify the parameters `'kernel'`, `'weights'` and
        `'delays'` of the connection dictionary.
    nest.GetConnections : Retrieve connections.


    Other parameters
    ----------------
    Available keys for the layer-specifying dictionary `projections`
    allow_autapses : bool, optional, default: True
        An autapse is a synapse (connection) from a node onto itself.
        It is used together with the `'number_of_connections'` option.
    allow_multapses : bool, optional, default: True
        Node A is connected to node B by a multapse if there are synapses
        (connections) from A to B.
        It is used together with the `'number_of_connections'` option.
    connection_type : str
        The type of connections can be either `'convergent'` or
        `'divergent'`. In case of convergent connections, the target
        layer is considered as driver layer and the source layer as pool
        layer - and vice versa for divergent connections.
    delays : [float | dict | Parameter object], optional, default: 1.0
        Delays can be constant, randomized or distance-dependent according
        to a provided function.
        Information on available functions can be found in the
        documentation on the function ``CreateParameter``.
    kernel : [float | dict | Parameter object], optional, default: 1.0
        A kernel is a function mapping the distance (or displacement)
        between a driver and a pool node to a connection probability. The
        default kernel is 1.0, i.e., connections are created with
        certainty.
        Information on available functions can be found in the
        documentation on the function ``CreateParameter``.
    mask : [dict | Mask object], optional
        The mask defines which pool nodes are considered as potential
        targets for each driver node. Parameters of the different
        available masks in 2 and 3 dimensions are also defined in
        dictionaries.
        If no mask is specified, all neurons from the pool layer are
        possible targets for each driver node.
        Information on available masks can be found in the documentation on
        the function ``CreateMask``.
    number_of_connections : int, optional
        Prescribed number of connections for each driver node. The actual
        connections being created are picked at random from all the
        candidate connections.
    synapse_model : str, optional
        The default synapse model in NEST is used if not specified
        otherwise.
    weights : [float | dict | Parameter object], optional, default: 1.0
        Weights can be constant, randomized or distance-dependent according
        to a provided function.
        Information on available functions can be found in the
        documentation on the function ``CreateParameter``.


    Notes
    -----

    * In the case of free probabilistic connections (in contrast to
      prescribing the number of connections), each possible driver-pool
      pair is inspected exactly once so that there will be at most one
      connection between each driver-pool pair.
    * Periodic boundary conditions are always applied in the pool layer.
      It is irrelevant whether the driver layer has periodic boundary
      conditions or not.
    * By default, Topology does not accept masks that are wider than the
      pool layer when using periodic boundary conditions.
      Kernel, weight and delay functions always consider the shortest
      distance (displacement) between driver and pool node.


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # connectivity specifications with a mask
            conndict1 = {'connection_type': 'divergent',
                         'mask': {'rectangular': {'lower_left'  : [-2.0, -1.0],
                                                  'upper_right' : [2.0, 1.0]}}}

            # connect layer l with itself according to the given
            # specifications
            tp.ConnectLayers(l, l, conndict1)


            # connection dictionary with distance-dependent kernel
            # (given as Parameter object) and randomized weights
            # (given as a dictionary)
            gauss_kernel = tp.CreateParameter('gaussian', {'p_center' : 1.0,
                                                           'sigma'    : 1.0})
            conndict2 = {'connection_type': 'divergent',
                         'mask': {'circular': {'radius': 2.0}},
                         'kernel': gauss_kernel,
                         'weights': {'uniform': {'min': 0.2, 'max': 0.8}}}
    """

    if not nest.is_sequence_of_gids(pre):
        raise TypeError("pre must be a sequence of GIDs")

    if not nest.is_sequence_of_gids(pre):
        raise TypeError("post must be a sequence of GIDs")

    if not len(pre) == len(post):
        raise nest.NESTError("pre and post must have the same length.")

    # ensure projections is list of full length
    projections = nest.broadcast(projections, len(pre), (dict, ),
                                 "projections")

    # Replace python classes with SLI datums
    def fixdict(d):
        d = d.copy()
        for k, v in d.items():
            if isinstance(v, dict):
                d[k] = fixdict(v)
            elif isinstance(v, Mask) or isinstance(v, Parameter):
                d[k] = v._datum
        return d

    projections = [fixdict(p) for p in projections]

    topology_func('3 arraystore { ConnectLayers } ScanThread', pre, post,
                  projections)


def GetPosition(nodes):
    """
    Return the spatial locations of nodes.


    Parameters
    ----------
    nodes : tuple/list of int(s)
        List of GIDs


    Returns
    -------
    out : tuple of tuple(s)
        List of positions as 2- or 3-element lists


    See also
    --------
    Displacement : Get vector of lateral displacement between nodes.
    Distance : Get lateral distance between nodes.
    DumpLayerConnections : Write connectivity information to file.
    DumpLayerNodes : Write layer node positions to file.


    Notes
    -----
    * The functions ``GetPosition``, ``Displacement`` and ``Distance`` now
      only works for nodes local to the current MPI process, if used in a
      MPI-parallel simulation.


    **Example**
        ::

            import nest
            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # retrieve positions of all (local) nodes belonging to the layer
            gids = nest.GetNodes(l, {'local_only': True})[0]
            tp.GetPosition(gids)
    """

    if not nest.is_sequence_of_gids(nodes):
        raise TypeError("nodes must be a sequence of GIDs")

    return topology_func('{ GetPosition } Map', nodes)


def GetLayer(nodes):
    """
    Return the layer to which nodes belong.


    Parameters
    ----------
    nodes : tuple/list of int(s)
        List of neuron GIDs


    Returns
    -------
    out : tuple of int(s)
        List of layer GIDs


    See also
    --------
    GetElement : Return the node(s) at the location(s) in the given layer(s).
    GetPosition : Return the spatial locations of nodes.


    Notes
    -----
    -


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # get layer GID of nodes in layer
            tp.GetLayer(nest.GetNodes(l)[0])
    """

    if not nest.is_sequence_of_gids(nodes):
        raise TypeError("nodes must be a sequence of GIDs")

    return topology_func('{ GetLayer } Map', nodes)


def GetElement(layers, locations):
    """
    Return the node(s) at the location(s) in the given layer(s).

    This function works for fixed grid layers only.

    * If layers contains a single GID and locations is a single 2-element
      array giving a grid location, return a list of GIDs of layer elements
      at the given location.
    * If layers is a list with a single GID and locations is a list of
      coordinates, the function returns a list of lists with GIDs of the nodes
      at all locations.
    * If layers is a list of GIDs and locations single 2-element array giving
      a grid location, the function returns a list of lists with the GIDs of
      the nodes in all layers at the given location.
    * If layers and locations are lists, it returns a nested list of GIDs, one
      list for each layer and each location.


    Parameters
    ----------
    layers : tuple/list of int(s)
        List of layer GIDs
    locations : [tuple/list of floats | tuple/list of tuples/lists of floats]
        2-element list with coordinates of a single grid location,
        or list of 2-element lists of coordinates for 2-dimensional layers,
        i.e., on the format [column, row]


    Returns
    -------
    out : tuple of int(s)
        List of GIDs


    See also
    --------
    GetLayer : Return the layer to which nodes belong.
    FindNearestElement: Return the node(s) closest to the location(s) in the
        given layer(s).
    GetPosition : Return the spatial locations of nodes.


    Notes
    -----
    -


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 5,
                                'columns'   : 4,
                                'elements'  : 'iaf_psc_alpha'})

            # get GID of element in last row and column
            tp.GetElement(l, [3, 4])
    """

    if not nest.is_sequence_of_gids(layers):
        raise TypeError("layers must be a sequence of GIDs")

    if not len(layers) > 0:
        raise nest.NESTError("layers cannot be empty")

    if not (nest.is_iterable(locations) and len(locations) > 0):
        raise nest.NESTError(
            "locations must be coordinate array or list of coordinate arrays")

    # ensure that all layers are grid-based, otherwise one ends up with an
    # incomprehensible error message
    try:
        topology_func('{ [ /topology [ /rows /columns ] ] get ; } forall',
                      layers)
    except:
        raise nest.NESTError(
            "layers must contain only grid-based topology layers")

    # SLI GetElement returns either single GID or list
    def make_tuple(x):
        if not nest.is_iterable(x):
            return (x, )
        else:
            return x

    if nest.is_iterable(locations[0]):

        # layers and locations are now lists
        nodes = topology_func(
            '/locs Set { /lyr Set locs { lyr exch GetElement } Map } Map',
            layers, locations)

        node_list = tuple(
            tuple(make_tuple(nodes_at_loc) for nodes_at_loc in nodes_in_lyr)
            for nodes_in_lyr in nodes)

    else:

        # layers is list, locations is a single location
        nodes = topology_func('/loc Set { loc GetElement } Map', layers,
                              locations)

        node_list = tuple(make_tuple(nodes_in_lyr) for nodes_in_lyr in nodes)

    # If only a single layer is given, un-nest list
    if len(layers) == 1:
        node_list = node_list[0]

    return node_list


def FindNearestElement(layers, locations, find_all=False):
    """
    Return the node(s) closest to the location(s) in the given layer(s).

    This function works for fixed grid layers only.

    * If layers contains a single GID and locations is a single 2-element
      array giving a grid location, return a list of GIDs of layer elements
      at the given location.
    * If layers is a list with a single GID and locations is a list of
      coordinates, the function returns a list of lists with GIDs of the nodes
      at all locations.
    * If layers is a list of GIDs and locations single 2-element array giving
      a grid location, the function returns a list of lists with the GIDs of
      the nodes in all layers at the given location.
    * If layers and locations are lists, it returns a nested list of GIDs, one
      list for each layer and each location.


    Parameters
    ----------
    layers : tuple/list of int(s)
        List of layer GIDs
    locations : tuple(s)/list(s) of tuple(s)/list(s)
        2-element list with coordinates of a single position, or list of
        2-element list of positions
    find_all : bool, default: False
        If there are several nodes with same minimal distance, return only the
        first found, if `False`.
        If `True`, instead of returning a single GID, return a list of GIDs
        containing all nodes with minimal distance.


    Returns
    -------
    out : tuple of int(s)
        List of node GIDs


    See also
    --------
    FindCenterElement : Return GID(s) of node closest to center of layers.
    GetElement : Return the node(s) at the location(s) in the given layer(s).
    GetPosition : Return the spatial locations of nodes.


    Notes
    -----
    -


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # get GID of element closest to some location
            tp.FindNearestElement(l, [3.0, 4.0], True)
    """

    import numpy

    if not nest.is_sequence_of_gids(layers):
        raise TypeError("layers must be a sequence of GIDs")

    if not len(layers) > 0:
        raise nest.NESTError("layers cannot be empty")

    if not nest.is_iterable(locations):
        raise TypeError(
            "locations must be coordinate array or list of coordinate arrays")

    # ensure locations is sequence, keeps code below simpler
    if not nest.is_iterable(locations[0]):
        locations = (locations, )

    result = []  # collect one list per layer
    # loop over layers
    for lyr in layers:
        els = nest.GetChildren((lyr, ))[0]

        lyr_result = []
        # loop over locations
        for loc in locations:
            d = Distance(numpy.array(loc), els)

            if not find_all:
                dx = numpy.argmin(d)  # finds location of one minimum
                lyr_result.append(els[dx])
            else:
                mingids = list(els[:1])
                minval = d[0]
                for idx in range(1, len(els)):
                    if d[idx] < minval:
                        mingids = [els[idx]]
                        minval = d[idx]
                    elif numpy.abs(d[idx] - minval) <= 1e-14 * minval:
                        mingids.append(els[idx])
                lyr_result.append(tuple(mingids))
        result.append(tuple(lyr_result))

    # If both layers and locations are multi-element lists, result shall remain
    # a nested list. Otherwise, either the top or the second level is a single
    # element list and we flatten.
    assert (len(result) > 0)
    if len(result) == 1:
        assert (len(layers) == 1)
        return result[0]
    elif len(result[0]) == 1:
        assert (len(locations) == 1)
        return tuple(el[0] for el in result)
    else:
        return tuple(result)


def _check_displacement_args(from_arg, to_arg, caller):
    """
    Internal helper function to check arguments to Displacement
    and Distance and make them lists of equal length.
    """

    import numpy

    if isinstance(from_arg, numpy.ndarray):
        from_arg = (from_arg, )
    elif not (nest.is_iterable(from_arg) and len(from_arg) > 0):
        raise nest.NESTError(
            "%s: from_arg must be lists of GIDs or positions" % caller)
    # invariant: from_arg is list

    if not nest.is_sequence_of_gids(to_arg):
        raise nest.NESTError("%s: to_arg must be lists of GIDs" % caller)
    # invariant: from_arg and to_arg are sequences

    if len(from_arg) > 1 and len(to_arg) > 1 and not len(from_arg) == len(
            to_arg):
        raise nest.NESTError(
            "%s: If to_arg and from_arg are lists, they must have same length."
            % caller)
    # invariant: from_arg and to_arg have equal length,
    # or (at least) one has length 1

    if len(from_arg) == 1:
        from_arg = from_arg * len(to_arg)  # this is a no-op if len(to_arg)==1
    if len(to_arg) == 1:
        to_arg = to_arg * len(from_arg)  # this is a no-op if len(from_arg)==1
    # invariant: from_arg and to_arg have equal length

    return from_arg, to_arg


def Displacement(from_arg, to_arg):
    """
    Get vector of lateral displacement from node(s) `from_arg`
    to node(s) `to_arg`.

    Displacement is always measured in the layer to which the `to_arg` node
    belongs. If a node in the `from_arg` list belongs to a different layer,
    its location is projected into the `to_arg` layer. If explicit positions
    are given in the `from_arg` list, they are interpreted in the `to_arg`
    layer.
    Displacement is the shortest displacement, taking into account
    periodic boundary conditions where applicable.

    * If one of `from_arg` or `to_arg` has length 1, and the other is longer,
      the displacement from/to the single item to all other items is given.
    * If `from_arg` and `to_arg` both have more than two elements, they have
      to be lists of the same length and the displacement for each pair is
      returned.


    Parameters
    ----------
    from_arg : [tuple/list of int(s) | tuple/list of tuples/lists of floats]
        List of GIDs or position(s)
    to_arg : tuple/list of int(s)
        List of GIDs


    Returns
    -------
    out : tuple
        Displacement vectors between pairs of nodes in `from_arg` and `to_arg`


    See also
    --------
    Distance : Get lateral distances between nodes.
    DumpLayerConnections : Write connectivity information to file.
    GetPosition : Return the spatial locations of nodes.


    Notes
    -----
    * The functions ``GetPosition``, ``Displacement`` and ``Distance`` now
      only works for nodes local to the current MPI process, if used in a
      MPI-parallel simulation.


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # displacement between node 2 and 3
            print(tp.Displacement([2], [3]))

            # displacment between the position (0.0., 0.0) and node 2
            print(tp.Displacement([(0.0, 0.0)], [2]))
    """

    from_arg, to_arg = _check_displacement_args(from_arg, to_arg,
                                                'Displacement')
    return topology_func('{ Displacement } MapThread', [from_arg, to_arg])


def Distance(from_arg, to_arg):
    """
    Get lateral distances from node(s) from_arg to node(s) to_arg.

    The distance between two nodes is the length of its displacement.

    Distance is always measured in the layer to which the `to_arg` node
    belongs. If a node in the `from_arg` list belongs to a different layer,
    its location is projected into the `to_arg` layer. If explicit positions
    are given in the `from_arg` list, they are interpreted in the `to_arg`
    layer.
    Distance is the shortest distance, taking into account periodic boundary
    conditions where applicable.

    * If one of `from_arg` or `to_arg` has length 1, and the other is longer,
      the displacement from/to the single item to all other items is given.
    * If `from_arg` and `to_arg` both have more than two elements, they have
      to be lists of the same length and the distance for each pair is
      returned.


    Parameters
    ----------
    from_arg : [tuple/list of ints | tuple/list with tuples/lists of floats]
        List of GIDs or position(s)
    to_arg : tuple/list of ints
        List of GIDs


    Returns
    -------
    out : tuple
        Distances between from and to


    See also
    --------
    Displacement : Get vector of lateral displacements between nodes.
    DumpLayerConnections : Write connectivity information to file.
    GetPosition : Return the spatial locations of nodes.


    Notes
    -----
    * The functions ``GetPosition``, ``Displacement`` and ``Distance`` now
      only works for nodes local to the current MPI process, if used in a
      MPI-parallel simulation.


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # distance between node 2 and 3
            print(tp.Distance([2], [3]))

            # distance between the position (0.0., 0.0) and node 2
            print(tp.Distance([(0.0, 0.0)], [2]))

    """

    from_arg, to_arg = _check_displacement_args(from_arg, to_arg, 'Distance')
    return topology_func('{ Distance } MapThread', [from_arg, to_arg])


def _rank_specific_filename(basename):
    """Returns file name decorated with rank."""

    if nest.NumProcesses() == 1:
        return basename
    else:
        np = nest.NumProcesses()
        np_digs = len(str(np - 1))  # for pretty formatting
        rk = nest.Rank()
        dot = basename.find('.')
        if dot < 0:
            return '%s-%0*d' % (basename, np_digs, rk)
        else:
            return '%s-%0*d%s' % (basename[:dot], np_digs, rk, basename[dot:])


def DumpLayerNodes(layers, outname):
    """
    Write GID and position data of layer(s) to file.

    Write GID and position data to layer(s) file. For each node in a layer,
    a line with the following information is written:
        ::

            GID x-position y-position [z-position]

    If `layers` contains several GIDs, data for all layers will be written to a
    single file.


    Parameters
    ----------
    layers : tuple/list of int(s)
        List of GIDs of a Topology layer
    outname : str
        Name of file to write to (existing files are overwritten)


    Returns
    -------
    out : None


    See also
    --------
    DumpLayerConnections : Write connectivity information to file.
    GetPosition : Return the spatial locations of nodes.


    Notes
    -----
    * If calling this function from a distributed simulation, this function
      will write to one file per MPI rank.
    * File names are formed by adding the MPI Rank into the file name before
      the file name suffix.
    * Each file stores data for nodes local to that file.


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'     : 5,
                                'columns'  : 5,
                                'elements' : 'iaf_psc_alpha'})

            # write layer node positions to file
            tp.DumpLayerNodes(l, 'positions.txt')

    """
    topology_func("""
                  (w) file exch { DumpLayerNodes } forall close
                  """,
                  layers, _rank_specific_filename(outname))


def DumpLayerConnections(layers, synapse_model, outname):
    """
    Write connectivity information to file.

    This function writes connection information to file for all outgoing
    connections from the given layers with the given synapse model.
    Data for all layers in the list is combined.

    For each connection, one line is stored, in the following format:
        ::

            source_gid target_gid weight delay dx dy [dz]

    where (dx, dy [, dz]) is the displacement from source to target node.
    If targets do not have positions (eg spike detectors outside any layer),
    NaN is written for each displacement coordinate.


    Parameters
    ----------
    layers : tuple/list of int(s)
        List of GIDs of a Topology layer
    synapse_model : str
        NEST synapse model
    outname : str
        Name of file to write to (will be overwritten if it exists)


    Returns
    -------
    out : None


    See also
    --------
    DumpLayerNodes : Write layer node positions to file.
    GetPosition : Return the spatial locations of nodes.
    nest.GetConnections : Return connection identifiers between
        sources and targets


    Notes
    -----
    * If calling this function from a distributed simulation, this function
      will write to one file per MPI rank.
    * File names are formed by inserting
      the MPI Rank into the file name before the file name suffix.
    * Each file stores data for local nodes.


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})
            tp.ConnectLayers(l,l, {'connection_type': 'divergent',
                                   'synapse_model': 'static_synapse'})

            # write connectivity information to file
            tp.DumpLayerConnections(l, 'static_synapse', 'connections.txt')
    """

    topology_func("""
                  /oname  Set
                  cvlit /synmod Set
                  /lyrs   Set
                  oname (w) file lyrs
                  { synmod DumpLayerConnections } forall close
                  """,
                  layers, synapse_model, _rank_specific_filename(outname))


def FindCenterElement(layers):
    """
    Return GID(s) of node closest to center of layers.


    Parameters
    ----------
    layers : tuple/list of int(s)
        List of layer GIDs


    Returns
    -------
    out : tuple of int(s)
        A list containing for each layer the GID of the node closest to the
        center of the layer, as specified in the layer parameters. If several
        nodes are equally close to the center, an arbitrary one of them is
        returned.


    See also
    --------
    FindNearestElement : Return the node(s) closest to the location(s) in the
        given layer(s).
    GetElement : Return the node(s) at the location(s) in the given layer(s).
    GetPosition : Return the spatial locations of nodes.


    Notes
    -----
    -


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # get GID of the element closest to the center of the layer
            tp.FindCenterElement(l)
    """

    if not nest.is_sequence_of_gids(layers):
        raise TypeError("layers must be a sequence of GIDs")

    # Do each layer on its own since FindNearestElement does not thread
    return tuple(FindNearestElement((lyr, ),
                                    nest.GetStatus((lyr, ), 'topology')[0][
                                        'center'])[0]
                 for lyr in layers)


def GetTargetNodes(sources, tgt_layer, tgt_model=None, syn_model=None):
    """
    Obtain targets of a list of sources in given target layer.


    Parameters
    ----------
    sources : tuple/list of int(s)
        List of GID(s) of source neurons
    tgt_layer : tuple/list of int(s)
        Single-element list with GID of tgt_layer
    tgt_model : [None | str], optional, default: None
        Return only target positions for a given neuron model.
    syn_model : [None | str], optional, default: None
        Return only target positions for a given synapse model.


    Returns
    -------
    out : tuple of list(s) of int(s)
        List of GIDs of target neurons fulfilling the given criteria.
        It is a list of lists, one list per source.

        For each neuron in `sources`, this function finds all target elements
        in `tgt_layer`. If `tgt_model` is not given (default), all targets are
        returned, otherwise only targets of specific type, and similarly for
        syn_model.


    See also
    --------
    GetTargetPositions : Obtain positions of targets of a list of sources in a
        given target layer.
    nest.GetConnections : Return connection identifiers between
        sources and targets


    Notes
    -----
    * For distributed simulations, this function only returns targets on the
      local MPI process.


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # connectivity specifications with a mask
            conndict = {'connection_type': 'divergent',
                        'mask': {'rectangular': {'lower_left' : [-2.0, -1.0],
                                                 'upper_right': [2.0, 1.0]}}}

            # connect layer l with itself according to the given
            # specifications
            tp.ConnectLayers(l, l, conndict)

            # get the GIDs of the targets of the source neuron with GID 5
            tp.GetTargetNodes([5], l)
    """

    if not nest.is_sequence_of_gids(sources):
        raise TypeError("sources must be a sequence of GIDs")

    if not nest.is_sequence_of_gids(tgt_layer):
        raise TypeError("tgt_layer must be a sequence of GIDs")

    if len(tgt_layer) != 1:
        raise nest.NESTError("tgt_layer must be a one-element list")

    with nest.SuppressedDeprecationWarning('GetLeaves'):
        # obtain local nodes in target layer, to pass to GetConnections
        tgt_nodes = nest.GetLeaves(tgt_layer,
                                   properties={'model': tgt_model}
                                   if tgt_model else None,
                                   local_only=True)[0]

    conns = nest.GetConnections(sources, tgt_nodes, synapse_model=syn_model)

    # conns is a flat list of connections.
    # Re-organize into one list per source, containing only target GIDs.
    src_tgt_map = dict((sgid, []) for sgid in sources)
    for conn in conns:
        src_tgt_map[conn[0]].append(conn[1])

    # convert dict to nested list in same order as sources
    return tuple(src_tgt_map[sgid] for sgid in sources)


def GetTargetPositions(sources, tgt_layer, tgt_model=None, syn_model=None):
    """
    Obtain positions of targets of a list of sources in a given target layer.


    Parameters
    ----------
    sources : tuple/list of int(s)
        List of GID(s) of source neurons
    tgt_layer : tuple/list of int(s)
        Single-element list with GID of tgt_layer
    tgt_model : [None | str], optional, default: None
        Return only target positions for a given neuron model.
    syn_type : [None | str], optional, default: None
        Return only target positions for a given synapse model.


    Returns
    -------
    out : tuple of tuple(s) of tuple(s) of floats
        Positions of target neurons fulfilling the given criteria as a nested
        list, containing one list of positions per node in sources.

        For each neuron in `sources`, this function finds all target elements
        in `tgt_layer`. If `tgt_model` is not given (default), all targets are
        returned, otherwise only targets of specific type, and similarly for
        syn_model.


    See also
    --------
    GetTargetNodes : Obtain targets of a list of sources in a given target
        layer.


    Notes
    -----
    * For distributed simulations, this function only returns targets on the
      local MPI process.


    **Example**
        ::

            import nest.topology as tp

            # create a layer
            l = tp.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # connectivity specifications with a mask
            conndict1 = {'connection_type': 'divergent',
                         'mask': {'rectangular': {'lower_left'  : [-2.0, -1.0],
                                                  'upper_right' : [2.0, 1.0]}}}

            # connect layer l with itself according to the given
            # specifications
            tp.ConnectLayers(l, l, conndict1)

            # get the positions of the targets of the source neuron with GID 5
            tp.GetTargetPositions([5], l)
    """

    return tuple(GetPosition(nodes) for nodes
                 in GetTargetNodes(sources, tgt_layer, tgt_model, syn_model))


def _draw_extent(ax, xctr, yctr, xext, yext):
    """Draw extent and set aspect ration, limits"""

    import matplotlib.pyplot as plt

    # thin gray line indicating extent
    llx, lly = xctr - xext / 2.0, yctr - yext / 2.0
    urx, ury = llx + xext, lly + yext
    ax.add_patch(
        plt.Rectangle((llx, lly), xext, yext, fc='none', ec='0.5', lw=1,
                      zorder=1))

    # set limits slightly outside extent
    ax.set(aspect='equal',
           xlim=(llx - 0.05 * xext, urx + 0.05 * xext),
           ylim=(lly - 0.05 * yext, ury + 0.05 * yext),
           xticks=tuple(), yticks=tuple())


def PlotLayer(layer, fig=None, nodecolor='b', nodesize=20):
    """
    Plot all nodes in a layer.

    This function plots only top-level nodes, not the content of composite
    nodes.


    Parameters
    ----------
    layer : tuple/list of int(s)
        GID of layer to plot, must be tuple/list of length 1
    fig : [None | matplotlib.figure.Figure object], optional, default: None
        Matplotlib figure to plot to. If not given, a new figure is
        created.
    nodecolor : [None | any matplotlib color], optional, default: 'b'
        Color for nodes
    nodesize : float, optional, default: 20
        Marker size for nodes


    Returns
    -------
    out : `matplotlib.figure.Figure` object


    See also
    --------
    PlotKernel : Add indication of mask and kernel to axes.
    PlotTargets : Plot all targets of a given source.
    matplotlib.figure.Figure : matplotlib Figure class


    Notes
    -----
    * Do not use this function in distributed simulations.


    **Example**
        ::

            import nest.topology as tp
            import matplotlib.pyplot as plt

            # create a layer
            l = tp.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # plot layer with all its nodes
            tp.PlotLayer(l)
            plt.show()
    """

    import matplotlib.pyplot as plt

    if len(layer) != 1:
        raise ValueError("layer must contain exactly one GID.")

    # get layer extent
    ext = nest.GetStatus(layer, 'topology')[0]['extent']

    if len(ext) == 2:
        # 2D layer

        # get layer extent and center, x and y
        xext, yext = ext
        xctr, yctr = nest.GetStatus(layer, 'topology')[0]['center']

        with nest.SuppressedDeprecationWarning('GetChildren'):
            # extract position information, transpose to list of x and y pos
            xpos, ypos = zip(*GetPosition(nest.GetChildren(layer)[0]))

        if fig is None:
            fig = plt.figure()
            ax = fig.add_subplot(111)
        else:
            ax = fig.gca()

        ax.scatter(xpos, ypos, s=nodesize, facecolor=nodecolor,
                   edgecolor='none')
        _draw_extent(ax, xctr, yctr, xext, yext)

    elif len(ext) == 3:
        # 3D layer
        from mpl_toolkits.mplot3d import Axes3D

        with nest.SuppressedDeprecationWarning('GetChildren'):
            # extract position information, transpose to list of x,y,z pos
            pos = zip(*GetPosition(nest.GetChildren(layer)[0]))

        if fig is None:
            fig = plt.figure()
            ax = fig.add_subplot(111, projection='3d')
        else:
            ax = fig.gca()

        ax.scatter3D(*pos, s=nodesize, facecolor=nodecolor, edgecolor='none')
        plt.draw_if_interactive()

    else:
        raise nest.NESTError("unexpected dimension of layer")

    return fig


def PlotTargets(src_nrn, tgt_layer, tgt_model=None, syn_type=None, fig=None,
                mask=None, kernel=None,
                src_color='red', src_size=50, tgt_color='blue', tgt_size=20,
                mask_color='red', kernel_color='red'):
    """
    Plot all targets of source neuron `src_nrn` in a target layer `tgt_layer`.


    Parameters
    ----------
    src_nrn : int
        GID of source neuron (as single-element list)
    tgt_layer : tuple/list of int(s)
        GID of tgt_layer (as single-element list)
    tgt_model : [None | str], optional, default: None
        Show only targets of a given model.
    syn_type : [None | str], optional, default: None
        Show only targets connected to with a given synapse type
    fig : [None | matplotlib.figure.Figure object], optional, default: None
        Matplotlib figure to plot to. If not given, a new figure is created.
    mask : [None | dict], optional, default: None
        Draw topology mask with targets; see ``PlotKernel`` for details.
    kernel : [None | dict], optional, default: None
        Draw topology kernel with targets; see ``PlotKernel`` for details.
    src_color : [None | any matplotlib color], optional, default: 'red'
        Color used to mark source node position
    src_size : float, optional, default: 50
        Size of source marker (see scatter for details)
    tgt_color : [None | any matplotlib color], optional, default: 'blue'
        Color used to mark target node positions
    tgt_size : float, optional, default: 20
        Size of target markers (see scatter for details)
    mask_color : [None | any matplotlib color], optional, default: 'red'
        Color used for line marking mask
    kernel_color : [None | any matplotlib color], optional, default: 'red'
        Color used for lines marking kernel


    Returns
    -------
    out : matplotlib.figure.Figure object


    See also
    --------
    GetTargetNodes : Obtain targets of a list of sources in a given target
        layer.
    GetTargetPositions : Obtain positions of targets of a list of sources in a
        given target layer.
    PlotKernel : Add indication of mask and kernel to axes.
    PlotLayer : Plot all nodes in a layer.
    matplotlib.pyplot.scatter : matplotlib scatter plot.


    Notes
    -----
    * Do not use this function in distributed simulations.


    **Example**
        ::

            import nest.topology as tp
            import matplotlib.pyplot as plt

            # create a layer
            l = tp.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # connectivity specifications with a mask
            conndict = {'connection_type': 'divergent',
                         'mask': {'rectangular': {'lower_left'  : [-2.0, -1.0],
                                                  'upper_right' : [2.0, 1.0]}}}

            # connect layer l with itself according to the given
            # specifications
            tp.ConnectLayers(l, l, conndict)

            # plot the targets of the source neuron with GID 5
            tp.PlotTargets([5], l)
            plt.show()
    """

    import matplotlib.pyplot as plt

    # get position of source
    srcpos = GetPosition(src_nrn)[0]

    # get layer extent and center, x and y
    ext = nest.GetStatus(tgt_layer, 'topology')[0]['extent']

    if len(ext) == 2:
        # 2D layer

        # get layer extent and center, x and y
        xext, yext = ext
        xctr, yctr = nest.GetStatus(tgt_layer, 'topology')[0]['center']

        if fig is None:
            fig = plt.figure()
            ax = fig.add_subplot(111)
        else:
            ax = fig.gca()

        # get positions, reorganize to x and y vectors
        tgtpos = GetTargetPositions(src_nrn, tgt_layer, tgt_model, syn_type)
        if tgtpos:
            xpos, ypos = zip(*tgtpos[0])
            ax.scatter(xpos, ypos, s=tgt_size, facecolor=tgt_color,
                       edgecolor='none')

        ax.scatter(srcpos[:1], srcpos[1:], s=src_size, facecolor=src_color,
                   edgecolor='none',
                   alpha=0.4, zorder=-10)

        _draw_extent(ax, xctr, yctr, xext, yext)

        if mask is not None or kernel is not None:
            PlotKernel(ax, src_nrn, mask, kernel, mask_color, kernel_color)

    else:
        # 3D layer
        from mpl_toolkits.mplot3d import Axes3D

        if fig is None:
            fig = plt.figure()
            ax = fig.add_subplot(111, projection='3d')
        else:
            ax = fig.gca()

        # get positions, reorganize to x,y,z vectors
        tgtpos = GetTargetPositions(src_nrn, tgt_layer, tgt_model, syn_type)
        if tgtpos:
            xpos, ypos, zpos = zip(*tgtpos[0])
            ax.scatter3D(xpos, ypos, zpos, s=tgt_size, facecolor=tgt_color,
                         edgecolor='none')

        ax.scatter3D(srcpos[:1], srcpos[1:2], srcpos[2:], s=src_size,
                     facecolor=src_color, edgecolor='none',
                     alpha=0.4, zorder=-10)

    plt.draw_if_interactive()

    return fig


def PlotKernel(ax, src_nrn, mask, kern=None, mask_color='red',
               kernel_color='red'):
    """
    Add indication of mask and kernel to axes.

    Adds solid red line for mask. For doughnut mask show inner and outer line.
    If kern is Gaussian, add blue dashed lines marking 1, 2, 3 sigma.
    This function ignores periodic boundary conditions.
    Usually, this function is invoked by ``PlotTargets``.


    Parameters
    ----------
    ax : matplotlib.axes.AxesSubplot,
        subplot reference returned by PlotTargets
    src_nrn : int
        GID of source neuron  (as single element list), mask and kernel
        plotted relative to it
    mask : dict
        Mask used in creating connections.
    kern : [None | dict], optional, default: None
        Kernel used in creating connections
    mask_color : [None | any matplotlib color], optional, default: 'red'
        Color used for line marking mask
    kernel_color : [None | any matplotlib color], optional, default: 'red'
        Color used for lines marking kernel


    Returns
    -------
    out : None


    See also
    --------
    CreateMask : Create a ``Mask`` object. Documentation on available spatial
        masks.
    CreateParameter : Create a ``Parameter`` object. Documentation on available
        parameters for distance dependency and randomization.
    PlotLayer : Plot all nodes in a layer.


    Notes
    -----
    * Do not use this function in distributed simulations.


    **Example**
        ::

            import nest.topology as tp
            import matplotlib.pyplot as plt

            # create a layer
            l = tp.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # connectivity specifications
            mask_dict = {'rectangular': {'lower_left'  : [-2.0, -1.0],
                                         'upper_right' : [2.0, 1.0]}}
            kernel_dict = {'gaussian': {'p_center' : 1.0,
                                        'sigma'    : 1.0}}
            conndict = {'connection_type': 'divergent',
                        'mask'   : mask_dict,
                        'kernel' : kernel_dict}

            # connect layer l with itself according to the given
            # specifications
            tp.ConnectLayers(l, l, conndict)

            # set up figure
            fig, ax = plt.subplots()

            # plot layer nodes
            tp.PlotLayer(l, fig)

            # choose center element of the layer as source node
            ctr_elem = tp.FindCenterElement(l)

            # plot mask and kernel of the center element
            tp.PlotKernel(ax, ctr_elem, mask=mask_dict, kern=kernel_dict)
    """

    import matplotlib
    import matplotlib.pyplot as plt
    import numpy as np

    # minimal checks for ax having been created by PlotKernel
    if ax and not isinstance(ax, matplotlib.axes.Axes):
        raise ValueError('ax must be matplotlib.axes.Axes instance.')

    srcpos = np.array(GetPosition(src_nrn)[0])

    if 'anchor' in mask:
        offs = np.array(mask['anchor'])
    else:
        offs = np.array([0., 0.])

    if 'circular' in mask:
        r = mask['circular']['radius']
        ax.add_patch(plt.Circle(srcpos + offs, radius=r, zorder=-1000,
                                fc='none', ec=mask_color, lw=3))
    elif 'doughnut' in mask:
        r_in = mask['doughnut']['inner_radius']
        r_out = mask['doughnut']['outer_radius']
        ax.add_patch(plt.Circle(srcpos + offs, radius=r_in, zorder=-1000,
                                fc='none', ec=mask_color, lw=3))
        ax.add_patch(plt.Circle(srcpos + offs, radius=r_out, zorder=-1000,
                                fc='none', ec=mask_color, lw=3))
    elif 'rectangular' in mask:
        ll = mask['rectangular']['lower_left']
        ur = mask['rectangular']['upper_right']
        pos = srcpos + ll + offs

        if 'azimuth_angle' in mask['rectangular']:
            angle = mask['rectangular']['azimuth_angle']
            angle_rad = angle * np.pi / 180
            cs = np.cos([angle_rad])[0]
            sn = np.sin([angle_rad])[0]
            pos = [pos[0] * cs - pos[1] * sn,
                   pos[0] * sn + pos[1] * cs]
        else:
            angle = 0.0

        ax.add_patch(
            plt.Rectangle(pos, ur[0] - ll[0], ur[1] - ll[1], angle=angle,
                          zorder=-1000, fc='none', ec=mask_color, lw=3))
    elif 'elliptical' in mask:
        width = mask['elliptical']['major_axis']
        height = mask['elliptical']['minor_axis']
        if 'azimuth_angle' in mask['elliptical']:
            angle = mask['elliptical']['azimuth_angle']
        else:
            angle = 0.0
        if 'anchor' in mask['elliptical']:
            anchor = mask['elliptical']['anchor']
        else:
            anchor = [0., 0.]
        ax.add_patch(
            matplotlib.patches.Ellipse(srcpos + offs + anchor, width, height,
                                       angle=angle, zorder=-1000, fc='none',
                                       ec=mask_color, lw=3))
    else:
        raise ValueError(
            'Mask type cannot be plotted with this version of PyTopology.')

    if kern is not None and isinstance(kern, dict):
        if 'gaussian' in kern:
            sigma = kern['gaussian']['sigma']
            for r in range(3):
                ax.add_patch(plt.Circle(srcpos + offs, radius=(r + 1) * sigma,
                                        zorder=-1000,
                                        fc='none', ec=kernel_color, lw=3,
                                        ls='dashed'))
        else:
            raise ValueError('Kernel type cannot be plotted with this ' +
                             'version of PyTopology')

    plt.draw()


def SelectNodesByMask(layer, anchor, mask_obj):
    """
    Obtain the GIDs inside a masked area of a topology layer.

    The function finds and returns all the GIDs inside a given mask of a single
    layer. It works on both 2-dimensional and 3-dimensional masks and layers.
    All mask types are allowed, including combined masks.

    Parameters
    ----------
    layer : tuple/list of int
        List containing the single layer to select nodes from.
    anchor : tuple/list of double
        List containing center position of the layer. This is the point from
        where we start to search.
    mask_obj: object
        Mask object specifying chosen area.

    Returns
    -------
    out : list of int(s)
        GID(s) of nodes/elements inside the mask.
    """

    if len(layer) != 1:
        raise ValueError("layer must contain exactly one GID.")

    mask_datum = mask_obj._datum

    gid_list = topology_func('SelectNodesByMask', layer[0], anchor, mask_datum)

    return gid_list
