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
3.  GIDs are given as GIDCollection.
4.  Commands returning GIDs return them as tuples or GIDCollection.
5.  Other arguments can be

    * single items that are applied to all entries in a GID list
    * a list of the same length as the given GIDCollection where each item is
      matched with the pertaining GID.

    **Example**
        ::

            layer = CreateLayer({...})

    creates a layer and returns a GIDCollection with the GIDs in the layer.
        ::

            ConnectLayers(layer, layer, {...})

    connects `layer` to `layer` using a dictionary to specify the connections.

Functions in the Topology module:
  - CreateMask(masktype, specs, anchor=None)
  - CreateTopologyParameter(parametertype, specs)
  - CreateLayer(specs)
  - ConnectLayers(pre, post, projections)
  - Distance(from_arg, to_arg)
  - Displacement(from_arg, to_arg)
  - FindNearestElement(layers, locations, find_all=False)
  - DumpLayerNodes(layers, outname)
  - DumpLayerConnections(source_layer, target_layer, synapse_model, outname)
  - FindCenterElement(layers)
  - GetTargetNodes(sources, tgt_layer, syn_model=None)
  - GetTargetPositions(sources, tgt_layer, syn_model=None)
  - PlotLayer(layer, fig=None, nodecolor='b', nodesize=20)
  - PlotTargets(src_nrn, tgt_layer, ...)
  - PlotKernel(ax, src_nrn, mask,...')
  - SelectNodesByMask(layer, anchor, mask_obj)


:Authors:
    Kittel Austvoll,
    Hans Ekkehard Plesser,
    Hakon Enger
"""

import nest

# With '__all__' we provide an explicit index of this submodule.
__all__ = [
    'ConnectLayers',
    'CreateMask',
    'CreateTopologyParameter',
    'Displacement',
    'Distance',
    'DumpLayerConnections',
    'DumpLayerNodes',
    'FindCenterElement',
    'FindNearestElement',
    'GetPosition',
    'GetTargetNodes',
    'GetTargetPositions',
    'PlotKernel',
    'PlotLayer',
    'PlotTargets',
    'SelectNodesByMask',
]


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
        see **Mask types**.
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
    ConnectLayers: Connect two layers pairwise according to specified
        projections. ``Mask`` objects can be passed in a connection
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

            import nest

            # create a grid-based layer
            l = nest.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # create a circular mask
            m = nest.CreateMask('circular', {'radius': 0.2})

            # connectivity specifications
            conndict = {'connection_type': 'divergent',
                        'mask'           : m}

            # connect layer l with itself according to the specifications
            nest.ConnectLayers(l, l, conndict)

    """
    if anchor is None:
        return nest.ll_api.sli_func('CreateMask', {masktype: specs})
    else:
        return nest.ll_api.sli_func('CreateMask',
                                    {masktype: specs, 'anchor': anchor})


def CreateTopologyParameter(parametertype, specs):
    """
    Create a parameter for distance dependency or randomization.

    Parameters are (spatial) functions which are used when creating
    connections in the Topology module for distance dependency or
    randomization. This command creates a Parameter object which may be
    combined with other ``TopologyParameter`` objects using arithmetic
    operators. The parameter is specified in a dictionary.

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
        `'parametertype'`, see **Parameter types**.


    Returns
    -------
    out : ``TopologyParameter`` object


    See also
    --------
    ConnectLayers : Connect two layers pairwise according to
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

            import nest

            # create a grid-based layer
            l = nest.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # parameter for delay with linear distance dependency
            d = nest.CreateTopologyParameter('linear', {'a': 0.2,
                                              'c': 0.2})

            # connectivity specifications
            conndict = {'connection_type': 'divergent',
                        'delays': d}

            nest.ConnectLayers(l, l, conndict)

    """
    return nest.ll_api.sli_func('CreateTopologyParameter',
                                {parametertype: specs})


def ConnectLayers(pre, post, projections):
    """
    Pairwise connect of pre- and postsynaptic layers.

    `pre` and `post` must be GIDCollections. The GIDs
    must refer to layers created with ``CreateLayers``. Layers in `pre`
    and `post` are connected pairwise.

    `projections` is a single dictionary, and applies to all pre-post pairs.

    A minimal call of ``ConnectLayers`` expects a source layer `pre`, a
    target layer `post` and a connection dictionary `projections`
    containing at least the entry `'connection_type'` (either
    `'convergent'` or `'divergent'`).

    When connecting two layers, the driver layer is the one in which each node
    is considered in turn. The pool layer is the one from which nodes are
    chosen for each node in the driver layer.


    Parameters
    ----------
    pre : GIDCollection
        GIDCollection with GIDs of presynaptic layers (sources)
    post : GIDCollection
       GIDCollection with GIDs of postsynaptic layers (targets)
    projections : dict
        Dictionary specifying projection properties


    Returns
    -------
    out : None
        ConnectLayers returns `None`


    See also
    --------
    CreateLayer : Create a Topology layer(s).
    CreateMask : Create a ``Mask`` object. Documentation on available spatial
        masks. Masks can be used to specify the key `'mask'` of the
        connection dictionary.
    CreateTopologyParameter : Create a ``TopologyParameter`` object.
        Documentation on available parameters for distance dependency and
        randomization. Parameters can be used to specify the parameters
        `'kernel'`, `'weights'` and `'delays'` of the connection dictionary.
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
        documentation on the function ``CreateTopologyParameter``.
    kernel : [float | dict | Parameter object], optional, default: 1.0
        A kernel is a function mapping the distance (or displacement)
        between a driver and a pool node to a connection probability. The
        default kernel is 1.0, i.e., connections are created with
        certainty.
        Information on available functions can be found in the
        documentation on the function ``CreateTopologyParameter``.
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
        documentation on the function ``CreateTopologyParameter``.


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

            import nest

            # create a layer
            l = nest.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # connectivity specifications with a mask
            conndict1 = {'connection_type': 'divergent',
                         'mask': {'rectangular': {'lower_left'  : [-2.0, -1.0],
                                                  'upper_right' : [2.0, 1.0]}}}

            # connect layer l with itself according to the given
            # specifications
            nest.ConnectLayers(l, l, conndict1)


            # connection dictionary with distance-dependent kernel
            # (given as Parameter object) and randomized weights
            # (given as a dictionary)
            gauss_kernel = nest.CreateTopologyParameter(
                'gaussian', {'p_center': 1.0, 'sigma': 1.0})
            conndict2 = {'connection_type': 'divergent',
                         'mask': {'circular': {'radius': 2.0}},
                         'kernel': gauss_kernel,
                         'weights': {'uniform': {'min': 0.2, 'max': 0.8}}}
    """
    if not isinstance(pre, nest.GIDCollection):
        raise TypeError("pre must be a GIDCollection")

    if not isinstance(post, nest.GIDCollection):
        raise TypeError("post must be a GIDCollection")

    # Replace python classes with SLI datums
    def fixdict(d):
        d = d.copy()
        for k, v in d.items():
            if isinstance(v, dict):
                d[k] = fixdict(v)
            elif isinstance(v, nest.Mask) or isinstance(v, nest.Parameter):
                d[k] = v._datum
        return d

    projections = fixdict(projections)

    nest.ll_api.sli_func('ConnectLayers', pre, post, projections)


def GetPosition(nodes):
    """
    Return the spatial locations of nodes.


    Parameters
    ----------
    nodes : GIDCollection
        GIDCollection of nodes we want the positions to


    Returns
    -------
    out : tuple or tuple of tuple(s)
        Tuple of position with 2- or 3-elements or list of positions


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

            # Reset kernel
            nest.ResetKernel

            # create a layer
            l = nest.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # retrieve positions of all (local) nodes belonging to the layer
            pos = GetPosition(l)

            # retrieve positions of the first node in the layer
            pos = GetPosition(l[0])

            # retrieve positions of node 4
            pos = GetPosition(l[4:5])

            # retrieve positions of a subset of nodes in the layer
            pos = GetPosition(l[2:18])
    """
    if not isinstance(nodes, nest.GIDCollection):
        raise TypeError("nodes must be a layer GIDCollection")

    return nest.ll_api.sli_func('GetPosition', nodes)


def Displacement(from_arg, to_arg):
    """
    Get vector of lateral displacement from node(s)/Position(s) `from_arg`
    to node(s) `to_arg`.

    Displacement is the shortest displacement, taking into account
    periodic boundary conditions where applicable. If explicit positions
    are given in the `from_arg` list, they are interpreted in the `to_arg`
    layer.

    * If one of `from_arg` or `to_arg` has length 1, and the other is longer,
      the displacement from/to the single item to all other items is given.
    * If `from_arg` and `to_arg` both have more than two elements, they have
      to be GIDCollections of the same length and the displacement for each
      pair is returned.


    Parameters
    ----------
    from_arg : GIDCollection or tuple/list with tuple(s)/list(s) of floats
        GIDCollection of GIDs or tuple/list of position(s)
    to_arg : GIDCollection
        GIDCollection of GIDs


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

            import nest

            # create a layer
            l = nest.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # displacement between node 2 and 3
            print(Displacement(l[1], l[2]))

            # displacment between the position (0.0., 0.0) and node 2
            print(Displacement([(0.0, 0.0)], l[1:2]))
    """
    if not isinstance(to_arg, nest.GIDCollection):
        raise TypeError("to_arg must be a GIDCollection")

    import numpy

    if isinstance(from_arg, numpy.ndarray):
        from_arg = (from_arg, )

    if (len(from_arg) > 1 and len(to_arg) > 1 and not
            len(from_arg) == len(to_arg)):
        raise nest.kernel.NESTError(
            "to_arg and from_arg must have same size unless one have size 1.")

    return nest.ll_api.sli_func('Displacement', from_arg, to_arg)


def Distance(from_arg, to_arg):
    """
    Get lateral distances from node(s)/position(s) from_arg to node(s) to_arg.

    The distance between two nodes is the length of its displacement.

    If explicit positions are given in the `from_arg` list, they are
    interpreted in the `to_arg` layer. Distance is the shortest distance,
    taking into account periodic boundary conditions where applicable.

    * If one of `from_arg` or `to_arg` has length 1, and the other is longer,
      the displacement from/to the single item to all other items is given.
    * If `from_arg` and `to_arg` both have more than two elements, they have
      to be lists of the same length and the distance for each pair is
      returned.


    Parameters
    ----------
    from_arg : GIDCollection or tuple/list with tuple(s)/list(s) of floats
        GIDCollection of GIDs or tuple/list of position(s)
    to_arg : GIDCollection
        GIDCollection of GIDs


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

            import nest

            # create a layer
            l = nest.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # distance between node 2 and 3
            print(Distance(l[1], l[2]))

            # distance between the position (0.0., 0.0) and node 2
            print(Distance([(0.0, 0.0)], l[1:2]))

    """
    if not isinstance(to_arg, nest.GIDCollection):
        raise TypeError("to_arg must be a GIDCollection")

    import numpy

    if isinstance(from_arg, numpy.ndarray):
        from_arg = (from_arg, )

    if (len(from_arg) > 1 and len(to_arg) > 1 and not
            len(from_arg) == len(to_arg)):
        raise nest.kernel.NESTError(
            "to_arg and from_arg must have same size unless one have size 1.")

    return nest.ll_api.sli_func('Distance', from_arg, to_arg)


def FindNearestElement(layer, locations, find_all=False):
    """
    Return the node(s) closest to the location(s) in the given layer.

    This function works for fixed grid layer only.

    * If locations is a single 2-element array giving a grid location, return a
      list of GIDs of layer elements at the given location.
    * If locations is a list of coordinates, the function returns a list of
      lists with GIDs of the nodes at all locations.


    Parameters
    ----------
    layer : GIDCollection
        GIDCollection of layer GIDs
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
    GetPosition : Return the spatial locations of nodes.


    Notes
    -----
    -


    **Example**
        ::

            import nest

            # create a layer
            l = nest.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # get GID of element closest to some location
            nest.FindNearestElement(l, [3.0, 4.0], True)
    """

    import numpy

    if not isinstance(layer, nest.GIDCollection):
        raise TypeError("layer must be a GIDCollection")

    if not len(layer) > 0:
        raise nest.kernel.NESTError("layer cannot be empty")

    if not nest.hl_api.is_iterable(locations):
        raise TypeError(
            "locations must be coordinate array or list of coordinate arrays")

    # Ensure locations is sequence, keeps code below simpler
    if not nest.hl_api.is_iterable(locations[0]):
        locations = (locations, )

    result = []

    for loc in locations:
        d = Distance(numpy.array(loc), layer)

        if not find_all:
            dx = numpy.argmin(d)  # finds location of one minimum
            result.append(layer[dx].get('global_id'))
        else:
            mingids = list(layer[:1])
            minval = d[0]
            for idx in range(1, len(layer)):
                if d[idx] < minval:
                    mingids = [layer[idx].get('global_id')]
                    minval = d[idx]
                elif numpy.abs(d[idx] - minval) <= 1e-14 * minval:
                    mingids.append(layer[idx].get('global_id'))
            result.append(tuple(mingids))

    return tuple(result)


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


def DumpLayerNodes(layer, outname):
    """
    Write GID and position data of layer to file.

    Write GID and position data to layer file. For each node in a layer,
    a line with the following information is written:
        ::

            GID x-position y-position [z-position]

    If `layer` contains several GIDs, data for all layer will be written to a
    single file.


    Parameters
    ----------
    layer : GIDCollection
        GIDCollection of GIDs of a Topology layer
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

            import nest

            # create a layer
            l = nest.CreateLayer({'rows'     : 5,
                                'columns'  : 5,
                                'elements' : 'iaf_psc_alpha'})

            # write layer node positions to file
            nest.DumpLayerNodes(l, 'positions.txt')

    """
    if not isinstance(layer, nest.GIDCollection):
        raise TypeError("layer must be a GIDCollection")

    nest.ll_api.sli_func("""
                         (w) file exch DumpLayerNodes close
                         """,
                         layer, _rank_specific_filename(outname))


def DumpLayerConnections(source_layer, target_layer, synapse_model, outname):
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
    source_layers : GIDCollection
        GIDCollection of GIDs of a Topology layer
    target_layers : GIDCollection
        GIDCollection of GIDs of a Topology layer
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

            import nest

            # create a layer
            l = nest.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})
            nest.ConnectLayers(l,l, {'connection_type': 'divergent',
                                   'synapse_model': 'static_synapse'})

            # write connectivity information to file
            nest.DumpLayerConnections(l, l, 'static_synapse', 'connections.txt')
    """
    if not isinstance(source_layer, nest.GIDCollection):
        raise nest.kernel.NESTError("source_layer must be a GIDCollection")
    if not isinstance(target_layer, nest.GIDCollection):
        raise nest.kernel.NESTError("target_layer must be a GIDCollection")

    nest.ll_api.sli_func("""
                         /oname  Set
                         cvlit /synmod Set
                         /lyr_target Set
                         /lyr_source Set
                         oname (w) file lyr_target lyr_source synmod
                         DumpLayerConnections close
                         """,
                         source_layer, target_layer, synapse_model,
                         _rank_specific_filename(outname))


def FindCenterElement(layer):
    """
    Return GID(s) of node closest to center of layer.


    Parameters
    ----------
    layers : GIDCollection
        GIDCollection of layer GIDs


    Returns
    -------
    out : int
        The GID of the node closest to the center of the layer, as specified in
        the layer parameters. If several nodes are equally close to the center,
        an arbitrary one of them is returned.


    See also
    --------
    FindNearestElement : Return the node(s) closest to the location(s) in the
        given layer.
    GetPosition : Return the spatial locations of nodes.


    Notes
    -----
    -


    **Example**
        ::

            import nest

            # create a layer
            l = nest.CreateLayer({'rows'      : 5,
                                'columns'   : 5,
                                'elements'  : 'iaf_psc_alpha'})

            # get GID of the element closest to the center of the layer
            nest.FindCenterElement(l)
    """

    if not isinstance(layer, nest.GIDCollection):
        raise nest.kernel.NESTError("layer must be a GIDCollection")

    return FindNearestElement(layer, layer.spatial['center'])[0]


def GetTargetNodes(sources, tgt_layer, syn_model=None):
    """
    Obtain targets of a list of sources in given target layer.


    Parameters
    ----------
    sources : tuple/list of int(s)
        List of GID(s) of source neurons
    tgt_layer : GIDCollection
        GIDCollection with GIDs of tgt_layer
    syn_model : [None | str], optional, default: None
        Return only target positions for a given synapse model.


    Returns
    -------
    out : tuple of list(s) of int(s)
        List of GIDs of target neurons fulfilling the given criteria.
        It is a list of lists, one list per source.

        For each neuron in `sources`, this function finds all target elements
        in `tgt_layer`. If `syn_model` is not given (default), all targets are
        returned, otherwise only targets of specific type.


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

            import nest

            # create a layer
            l = nest.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # connectivity specifications with a mask
            conndict = {'connection_type': 'divergent',
                        'mask': {'rectangular': {'lower_left' : [-2.0, -1.0],
                                                 'upper_right': [2.0, 1.0]}}}

            # connect layer l with itself according to the given
            # specifications
            nest.ConnectLayers(l, l, conndict)

            # get the GIDs of the targets of the source neuron with GID 5
            nest.GetTargetNodes([5], l)
    """
    if not nest.hl_api.is_sequence_of_gids(sources):
        raise TypeError("sources must be a sequence of GIDs")

    if not isinstance(tgt_layer, nest.GIDCollection):
        raise nest.kernel.NESTError("tgt_layer must be a GIDCollection")

    conns = nest.GetConnections(sources, tgt_layer, synapse_model=syn_model)

    # Re-organize conns into one list per source, containing only target GIDs.
    src_tgt_map = dict((sgid, []) for sgid in sources)
    for src, tgt in zip(conns.source(), conns.target()):
        src_tgt_map[src].append(tgt)

    # convert dict to nested list in same order as sources
    return tuple(src_tgt_map[sgid] for sgid in sources)


def GetTargetPositions(sources, tgt_layer, syn_model=None):
    """
    Obtain positions of targets to a given GIDCollection of sources.


    Parameters
    ----------
    sources : GIDCollection
        GIDCollection with GID(s) of source neurons
    tgt_layer : GIDCollection
        GIDCollection of tgt_layer
    syn_type : [None | str], optional, default: None
        Return only target positions for a given synapse model.


    Returns
    -------
    out : list of list(s) of tuple(s) of floats
        Positions of target neurons fulfilling the given criteria as a nested
        list, containing one list of positions per node in sources.

        For each neuron in `sources`, this function finds all target elements
        in `tgt_layer`. If `syn_model` is not given (default), all targets are
        returned, otherwise only targets of specific type.


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

            import nest

            # create a layer
            l = nest.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # connectivity specifications with a mask
            conndict1 = {'connection_type': 'divergent',
                         'mask': {'rectangular': {'lower_left'  : [-2.0, -1.0],
                                                  'upper_right' : [2.0, 1.0]}}}

            # connect layer l with itself according to the given
            # specifications
            nest.ConnectLayers(l, l, conndict1)

            # get the positions of the targets of the source neuron with GID 5
            nest.GetTargetPositions(l[5:6], l)
    """
    if not isinstance(sources, nest.GIDCollection):
        raise ValueError("sources must be a GIDCollection.")

    # Find positions to all nodes in target layer
    pos_all_tgts = GetPosition(tgt_layer)
    first_tgt_gid = tgt_layer[0].get('global_id')

    connections = nest.GetConnections(sources, tgt_layer,
                                      synapse_model=syn_model)
    srcs = connections.get('source')
    tgts = connections.get('target')
    if isinstance(srcs, int):
        srcs = [srcs]
    if isinstance(tgts, int):
        tgts = [tgts]

    # Make dictionary where the keys are the source gids, which is mapped to a
    # list with the positions of the targets connected to the source.
    src_tgt_pos_map = dict((sgid, []) for sgid in sources)
    for i in range(len(connections)):
        tgt_indx = tgts[i] - first_tgt_gid
        src_tgt_pos_map[srcs[i]].append(pos_all_tgts[tgt_indx])

    # Turn dict into list in same order as sources
    return [src_tgt_pos_map[sgid] for sgid in sources]


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


def _shifted_positions(pos, ext):
    """Get shifted positions corresponding to boundary conditions."""
    return [[pos[0] + ext[0], pos[1]],
            [pos[0] - ext[0], pos[1]],
            [pos[0], pos[1] + ext[1]],
            [pos[0], pos[1] - ext[1]],
            [pos[0] + ext[0], pos[1] - ext[1]],
            [pos[0] - ext[0], pos[1] + ext[1]],
            [pos[0] + ext[0], pos[1] + ext[1]],
            [pos[0] - ext[0], pos[1] - ext[1]]]


def PlotLayer(layer, fig=None, nodecolor='b', nodesize=20):
    """
    Plot all nodes in a layer.

    This function plots only top-level nodes, not the content of composite
    nodes.


    Parameters
    ----------
    layer : GIDCollection (Layer)
        GIDCollection with GIDs of layer to plot
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

            import nest
            import matplotlib.pyplot as plt

            # create a layer
            l = nest.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # plot layer with all its nodes
            nest.PlotLayer(l)
            plt.show()
    """

    import matplotlib.pyplot as plt

    if not isinstance(layer, nest.GIDCollection):
        raise ValueError("layer must be a GIDCollection.")

    # get layer extent
    ext = layer.spatial['extent']

    if len(ext) == 2:
        # 2D layer

        # get layer extent and center, x and y
        xext, yext = ext
        xctr, yctr = layer.spatial['center']

        # extract position information, transpose to list of x and y pos
        xpos, ypos = zip(*GetPosition(layer))

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

        # extract position information, transpose to list of x,y,z pos
        pos = zip(*GetPosition(layer))

        if fig is None:
            fig = plt.figure()
            ax = fig.add_subplot(111, projection='3d')
        else:
            ax = fig.gca()

        ax.scatter3D(*pos, s=nodesize, facecolor=nodecolor, edgecolor='none')
        plt.draw_if_interactive()

    else:
        raise nest.kernel.NESTError("unexpected dimension of layer")

    return fig


def PlotTargets(src_nrn, tgt_layer, syn_type=None, fig=None,
                mask=None, kernel=None,
                src_color='red', src_size=50, tgt_color='blue', tgt_size=20,
                mask_color='red', kernel_color='red'):
    """
    Plot all targets of source neuron `src_nrn` in a target layer `tgt_layer`.


    Parameters
    ----------
    src_nrn : GIDCollection
        GIDCollection of source neuron (as single-element GIDCollection)
    tgt_layer : GIDCollection
        GIDCollection of tgt_layer
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

            import nest
            import matplotlib.pyplot as plt

            # create a layer
            l = nest.CreateLayer({'rows'      : 11,
                                'columns'   : 11,
                                'extent'    : [11.0, 11.0],
                                'elements'  : 'iaf_psc_alpha'})

            # connectivity specifications with a mask
            conndict = {'connection_type': 'divergent',
                         'mask': {'rectangular': {'lower_left'  : [-2.0, -1.0],
                                                  'upper_right' : [2.0, 1.0]}}}

            # connect layer l with itself according to the given
            # specifications
            nest.ConnectLayers(l, l, conndict)

            # plot the targets of the source neuron with GID 5
            nest.PlotTargets(l[4:5], l)
            plt.show()
    """

    import matplotlib.pyplot as plt

    if not isinstance(src_nrn, nest.GIDCollection) and len(src_nrn) != 1:
        raise ValueError("src_nrn must be a single element GIDCollection.")
    if not isinstance(tgt_layer, nest.GIDCollection):
        raise ValueError("tgt_layer must be a GIDCollection.")

    # get position of source
    srcpos = GetPosition(src_nrn)

    # get layer extent
    ext = tgt_layer.spatial['extent']

    if len(ext) == 2:
        # 2D layer

        # get layer extent and center, x and y
        xext, yext = ext
        xctr, yctr = tgt_layer.spatial['center']

        if fig is None:
            fig = plt.figure()
            ax = fig.add_subplot(111)
        else:
            ax = fig.gca()

        # get positions, reorganize to x and y vectors
        tgtpos = GetTargetPositions(src_nrn, tgt_layer, syn_type)
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
        tgtpos = GetTargetPositions(src_nrn, tgt_layer, syn_type)
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
    Usually, this function is invoked by ``PlotTargets``.


    Parameters
    ----------
    ax : matplotlib.axes.AxesSubplot,
        subplot reference returned by PlotTargets
    src_nrn : GIDCollection
        GIDCollection of source neuron (as single-element GIDCollection), mask
        and kernel plotted relative to it
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
    CreateTopologyParameter : Create a ``TopologyParameter`` object.
        Documentation on available parameters for distance dependency and
        randomization.
    PlotLayer : Plot all nodes in a layer.


    Notes
    -----
    * Do not use this function in distributed simulations.


    **Example**
        ::

            import nest
            import matplotlib.pyplot as plt

            # create a layer
            l = nest.CreateLayer({'rows'      : 11,
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
            nest.ConnectLayers(l, l, conndict)

            # set up figure
            fig, ax = plt.subplots()

            # plot layer nodes
            nest.PlotLayer(l, fig)

            # choose center element of the layer as source node
            ctr_elem = nest.FindCenterElement(l)

            # plot mask and kernel of the center element
            nest.PlotKernel(ax,
                l[ctr_elem],
                mask=mask_dict,
                kern=kernel_dict)
    """

    import matplotlib
    import matplotlib.pyplot as plt
    import numpy as np

    if not isinstance(src_nrn, nest.GIDCollection) and len(src_nrn) != 1:
        raise ValueError("src_nrn must be a single element GIDCollection.")

    # minimal checks for ax having been created by PlotKernel
    if ax and not isinstance(ax, matplotlib.axes.Axes):
        raise ValueError('ax must be matplotlib.axes.Axes instance.')

    srcpos = np.array(GetPosition(src_nrn))

    if 'anchor' in mask:
        offs = np.array(mask['anchor'])
    else:
        offs = np.array([0., 0.])

    src_nrn.set_spatial()
    periodic = src_nrn.spatial['edge_wrap']
    extent = src_nrn.spatial['extent']

    if 'circular' in mask:
        r = mask['circular']['radius']

        ax.add_patch(plt.Circle(srcpos + offs, radius=r, zorder=-1000,
                                fc='none', ec=mask_color, lw=3))

        if periodic:
            for pos in _shifted_positions(srcpos + offs, extent):
                ax.add_patch(plt.Circle(pos, radius=r, zorder=-1000,
                                        fc='none', ec=mask_color, lw=3))
    elif 'doughnut' in mask:
        r_in = mask['doughnut']['inner_radius']
        r_out = mask['doughnut']['outer_radius']
        ax.add_patch(plt.Circle(srcpos + offs, radius=r_in, zorder=-1000,
                                fc='none', ec=mask_color, lw=3))
        ax.add_patch(plt.Circle(srcpos + offs, radius=r_out, zorder=-1000,
                                fc='none', ec=mask_color, lw=3))

        if periodic:
            for pos in _shifted_positions(srcpos + offs, extent):
                ax.add_patch(plt.Circle(pos, radius=r_in, zorder=-1000,
                                        fc='none', ec=mask_color, lw=3))
                ax.add_patch(plt.Circle(pos, radius=r_out, zorder=-1000,
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

        if periodic:
            for pos in _shifted_positions(srcpos + ll + offs, extent):
                ax.add_patch(
                    plt.Rectangle(pos, ur[0] - ll[0], ur[1] - ll[1],
                                  angle=angle, zorder=-1000, fc='none',
                                  ec=mask_color, lw=3))
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

        if periodic:
            for pos in _shifted_positions(srcpos + offs + anchor, extent):
                ax.add_patch(
                    matplotlib.patches.Ellipse(pos, width, height, angle=angle,
                                               zorder=-1000, fc='none',
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

            if periodic:
                for pos in _shifted_positions(srcpos + offs, extent):
                    for r in range(3):
                        ax.add_patch(plt.Circle(pos, radius=(r + 1) * sigma,
                                                zorder=-1000, fc='none',
                                                ec=kernel_color, lw=3,
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
    layer : GIDCollection
        GIDCollection with GIDs of the layer to select nodes from.
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

    if not isinstance(layer, nest.GIDCollection):
        raise ValueError("layer must be a GIDCollection.")

    mask_datum = mask_obj._datum

    gid_list = nest.ll_api.sli_func('SelectNodesByMask',
                                    layer, anchor, mask_datum)

    return gid_list
