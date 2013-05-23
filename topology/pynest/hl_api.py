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
High-level API of PyNEST Topology Module.

This file defines the user-level functions of NEST's Python interface to the
Topology module. The basic approach is the same as for the PyNEST interface
to NEST:

1. Function names are the same as in SLI.

2. Nodes are identified by their GIDs.

3. GIDs are always given as lists.

4. Commands returning GIDs return them as lists.

5. Other arguments can either be single items, in which case
   they are applied to all entries in a GID list, or a list
   the same length as the GID lists given, in which case each
   item is matched with the pertaining GID.

   Example:

     layers = CreateLayers([{...}, {...}, {...}]

   creates three layers and returns an array of three GIDs.

     ConnectLayers(layers[:2], layers[1:], {...})

   connects layers[0]->layers[1] and layers[1]->layers[2] using
   the same dictionary to specify both connections.

     ConnectLayers(layers[:2], layers[1:], [{...}, {...}])

   connects the same layers, but the layers[0]->layers[1] connection
   is specified by the first dictionary, the layers[1]->layers[2]
   connection by the second.

Notes:
1. The semantics of the PyNEST Topology Module has changed significantly
   per with r89XX (Oct 2010). Previously, the module did not adhere to
   the "list of gids" semantics of PyNEST.

2. Some function names are now in plural form, e.g. CreateLayers.

Authors: Kittel Austvoll, Hans Ekkehard Plesser, Håkon Enger
"""

import types

nest = None

# provide all() for Python versions < 2.5, see #495
try:
    all([True])
except:
    def all(arr):
        """
        Return true if all elements of argument are true.
        Manual implementation for Python versions < 2.5.
        """
        return reduce(lambda a,b: a and b, arr, True)


def topology_func(slifunc, *args):
    """
    Execute slifunc with args in topology namespace.

    See also: nest.sli_func
    """

    # Execute function in topology namespace.
    # We need to pass the kwarg namespace as **{} instead of the usuual
    # way to keep Python 2.5 happy, see http://bugs.python.org/issue3473
    return nest.sli_func(slifunc, *args, **{'namespace': 'topology'})


class Mask(object):
    """Class for spatial masks.

    Masks are used when creating connections in the Topology module. A mask
    describes which area of the pool layer shall be searched for nodes to
    connect for any given node in the driver layer. Masks are created using
    the CreateMask command."""

    _datum = None

    # The constructor should not be called by the user
    def __init__(self,datum):
        """Masks must be created using the CreateMask command."""
        if not isinstance(datum,nest.Datum) or datum.type != "masktype":
            raise TypeError,"Expected mask Datum."
        self._datum=datum

    # Generic binary operation
    def _binop(self,op,other):
        if not isinstance(other,Mask):
            return NotImplemented
        return Mask(topology_func(op,self._datum,other._datum))

    def __or__(self,other): return self._binop("or",other)
    def __and__(self,other): return self._binop("and",other)
    def __sub__(self,other): return self._binop("sub",other)

    def Inside(self,point):
        """Test if a point is inside a mask.

        Returns
        -------
        True if the point is inside the mask, False otherwise.

        See also
        --------
        nest.help("topology::Inside")
        """
        return topology_func("Inside",point,self._datum)


def CreateMask(masktype,specs,anchor=None):
    """
    Create a spatial mask according the the given specifications.

    Parameters
    ----------
    specs: dict
    anchor: list

    Returns
    -------
    Mask object

    Dictionary must be valid mask specification.

    See also
    --------
    nest.help("topology::CreateMask")
    """
    if anchor is None:
        return Mask(topology_func('CreateMask',{masktype:specs}))
    else:
        return Mask(topology_func('CreateMask',{masktype:specs,'anchor':anchor}))


class Parameter(object):
    """Class for spatial parameters.

    Parameters are spatial functions which are used when creating
    connections in the Topology module. A parameter may be used as a
    probability kernel when creating connections or as synaptic parameters
    (such as weight and delay). Parameters are created using the
    CreateParameter command."""

    _datum = None

    # The constructor should not be called by the user
    def __init__(self,datum):
        """Parameters must be created using the CreateParameter command."""
        if not isinstance(datum,nest.Datum) or datum.type != "parametertype":
            raise TypeError,"Expected parameter Datum."
        self._datum=datum

    # Generic binary operation
    def _binop(self,op,other):
        if not isinstance(other,Parameter):
            return NotImplemented
        return Parameter(topology_func(op,self._datum,other._datum))

    def __add__(self,other): return self._binop("add",other)
    def __sub__(self,other): return self._binop("sub",other)
    def __mul__(self,other): return self._binop("mul",other)
    def __div__(self,other): return self._binop("div",other)
    def __truediv__(self,other): return self._binop("div",other)

    def GetValue(self,point):
        """Compute value of parameter at a point.

        Returns
        -------
        The value of the parameter at the point.

        See also
        --------
        nest.help("topology::GetValue")
        """
        return topology_func("GetValue",point,self._datum)


def CreateParameter(parametertype,specs):
    """
    Create a spatial parameter according the the given specifications.

    Parameters
    ----------
    specs: dict

    Returns
    -------
    Mask object

    Dictionary must be valid parameter specification.

    See also
    --------
    nest.help("topology::CreateParameter")
    """
    return Parameter(topology_func('CreateParameter',{parametertype:specs}))


def CreateLayer(specs):
    """
    Create a layer or layers according the the given specifications.

    Parameters
    ----------
    specs: dict or list of dicts

    Returns
    -------
    List of GIDs

    If specs is a dictionary, a single layer is created, if it is
    a list of dictionaries, one layer is created for each dictionary.
    Dictionaries must be valid topology layer specifications.

    See also
    --------
    nest.help("topology::CreateLayer")
    """

    # ensure we can work on list/tuple in any case
    if not nest.is_sequencetype(specs):
        specs = (specs,)

    # ensure all specs are dicts
    if not all([type(spec) == types.DictType for spec in specs]):
        raise nest.NESTError("specs must be dictionary of list of dictionaries")

    return topology_func('{ CreateLayer } Map', specs)


def ConnectLayers(pre, post, projections):
    """
    Connect two layers or two lists of layers pairwise according to the
    projections specified.

    Parameters
    ----------
    pre        : List of GIDs of presynaptic layers (sources)
    post       : List of GIDs of postsynaptic layers (targets)
    projections: Dictionary or list of dictionary specifying projection properties

    pre and post must be lists of GIDs of equal length, the GIDs must refer to layers
    created with CreateLayers. Layers in the pre and post lists are connected pairwise.
    If projections is a single dictionary, it applies to all pre-post pairs.
    If projections is a list of dictionaries, it must have the same length as pre and
    post and each dictionary is matched with the proper pre-post pair.

    Example
    -------
    ConnectLayers([1, 10], [50, 100], {...})         # 1 -> 50 and 10 -> 100, same dict
    ConnectLayers([1, 10], [50, 100], [{...},{...}]) # 1 -> 50 and 10 -> 100, diff. dicts
    
    See also
    --------
    nest.help("topology::ConnectLayers")
    """

    nest.raise_if_not_list_of_gids(pre, 'pre')
    nest.raise_if_not_list_of_gids(post, 'post')

    if not len(pre) == len(post):
        raise nest.NESTError("pre and post must have the same length.")

    # ensure projections is list of full length
    projections = nest.broadcast(projections, len(pre), (dict,), "projections")

    # Replace python classes with SLI datums
    def fixdict(d):
        d = d.copy()
        for k,v in d.items():
            if isinstance(v,types.DictType):
                d[k] = fixdict(v)
            elif isinstance(v,Mask) or isinstance(v,Parameter):
                d[k] = v._datum
        return d

    projections = [fixdict(p) for p in projections]

    topology_func('3 arraystore { ConnectLayers } ScanThread', pre, post, projections)


def GetPosition(nodes):
    """
    Return the spatial locations of nodes.

    Parameters
    ----------
    nodes: list of GIDs

    Returns
    -------
    List of positions as 2- or 3-element lists

    See also
    --------
    nest.help("topology::GetPosition")
    """

    nest.raise_if_not_list_of_gids(nodes, 'nodes')

    return topology_func('{ GetPosition } Map', nodes)


def GetLayer(nodes):
    """
    Return the layer to which nodes belong.

    Parameters
    ----------
    nodes: list of GIDs

    Returns
    -------
    List of GIDs

    See also
    --------
    nest.help("topology::GetLayer")
    """

    nest.raise_if_not_list_of_gids(nodes, 'nodes')

    return topology_func('{ GetLayer } Map', nodes)



def GetElement(layers, locations):
    """
    Return the node(s) at the location(s) in the given layer(s).

    Parameters
    ----------
    layers   : list of layer GIDs
    locations: 2-element array with coordinates of a single grid location,
               or list of 2-element arrays of coordinates

    Returns
    -------
    List of GIDs

    This function works for fixed grid layers only.

    If layers contains a single GID and locations is a single 2-element
    array giving a grid location, return a list of GIDs of layer elements
    at the given location.

    If layers is a list with a single GID and locations is a list of
    coordinates, the function returns a list of lists with GIDs of the
    nodes at all locations.

    If layers is a list of GIDs and locations single 2-element array giving
    a grid location, the function returns a list of lists with the GIDs of
    the nodes in all layers at the given location.

    If layers and locations are lists, it returns a nested list of GIDs,
    one list for each layer and each location.

    See also
    --------
    FindNearestElement
    nest.help("topology::GetElement")
    """

    nest.raise_if_not_list_of_gids(layers, 'layers')
    if not len(layers) > 0:
        raise nest.NESTError("layers cannot be empty")

    if not ( nest.is_sequencetype(locations) and len(locations) > 0 ):
        raise nest.NESTError("locations must be coordinate array or list of coordinate arrays")

    # ensure that all layers are grid-based, otherwise one ends up with an
    # incomprehensible error message
    try:
        topology_func('{ [ /topology [ /rows /columns ] ] get ; } forall', layers)
    except: 
        raise nest.NESTError("layers must contain only grid-based topology layers")
    
    # SLI GetElement returns either single GID or list
    def makelist(x):
        if not nest.is_sequencetype(x):
            return [x] 
        else:
            return x

    if nest.is_sequencetype(locations[0]):

        # layers and locations are now lists
        nodes = topology_func('/locs Set { /lyr Set locs { lyr exch GetElement } Map } Map',
                              layers, locations)

        node_list = [[makelist(nodes_at_loc) for nodes_at_loc in nodes_in_lyr]
                     for nodes_in_lyr in nodes]

    else:

        # layers is list, locations is a single location
        nodes = topology_func('/loc Set { loc GetElement } Map', layers, locations)

        node_list = [makelist(nodes_in_lyr) for nodes_in_lyr in nodes]

    # If only a single layer is given, un-nest list
    if len(layers)==1: node_list=node_list[0]

    return node_list


def FindNearestElement(layers, locations, find_all=False):
    """
    Return the node(s) closest to the location(s) in the given layer(s).

    Parameters
    ----------
    layers   : list of layer GIDs
    locations: 2-element array with coordinates of a single position,
               or list of 2-element arrays of positions
    find_all : Default value false: if there are several nodes with same
               minimal distance, return only the first found. If True,
               instead of returning a single GID, return a list of
               GIDs containing all nodes with minimal distance.           

    Returns
    -------
    List of GIDs

    If layers contains a single GID and locations is a single 2-element
    array giving a location, return single-element list with GID
    of layer element closest to the given location.
    
    If layers is a list with a single GID and locations is a list of coordinates,
    the function returns a list of GIDs of the nodes closest to all locations.

    If layers is a list of GIDs and locations single 2-element array giving
    a position location, the function returns a list with the GIDs of the nodes
    in all layers closest to the given location.

    If layers and locations are lists, it returns a list of lists of GIDs,
    one for each layer.

    See also
    --------
    GetElement
    """

    import numpy
    nest.raise_if_not_list_of_gids(layers, 'layers')
    if not len(layers) > 0:
        raise nest.NESTError("layers cannot be empty")

    if not ( nest.is_sequencetype(locations) and len(locations) > 0 ):
        raise nest.NESTError("locations must be coordinate array or list of coordinate arrays")

    # ensure locations is sequence, keeps code below simpler
    if not nest.is_sequencetype(locations[0]):
        locations = [locations]
   
    result = []  # collect one list per layer
    # loop over layers
    for lyr in layers:
        els = nest.GetChildren([lyr])[0]

        lyr_result = [] 
        # loop over locations
        for loc in locations:
            d = Distance(numpy.array(loc), els)
   
            if not find_all:
                dx = numpy.argmin(d)   # finds location of one minimum
                lyr_result.append(els[dx])
            else:
                mingids = els[:1]
                minval  = d[0]  
                for idx in xrange(1, len(els)):
                    if d[idx] < minval:
                        mingids = [els[idx]]
                        minval = d[idx]
                    elif numpy.abs(d[idx] - minval) <= 1e-14 * minval:
                        mingids.append(els[idx])
                lyr_result.append(mingids)   
        result.append(lyr_result)

    # If both layers and locations are multi-element lists, result shall remain a nested list
    # Otherwise, either the top or the second level is a single element list and we flatten
    assert(len(result) > 0)
    if len(result) == 1:
        assert(len(layers) == 1)
        return result[0]
    elif len(result[0]) == 1:
        assert(len(locations) == 1)
        return [el[0] for el in result]
    else:
        return result    


def _check_displacement_args(from_arg, to_arg, caller):
    """
    Internal helper function to check arguments to Displacement
    and Distance and make them lists of equal length.
    """

    import numpy
    if isinstance(from_arg, numpy.ndarray):
        from_arg = [from_arg]
    elif not nest.is_sequencetype(from_arg) and len(from_arg) > 0:
        raise nest.NESTError("%s: from_arg must be lists of GIDs or positions" % caller)
    # invariant: from_arg is list
    
    if not ( nest.is_sequencetype(to_arg) and len(to_arg) > 0 ):
        raise nest.NESTError("%s: to_arg must be lists of GIDs" % caller)
    # invariant: from_arg and to_arg are sequences
    
    if len(from_arg) > 1 and len(to_arg) > 1 and not len(from_arg) == len(to_arg):
        raise nest.NESTError("%s: If to_arg and from_arg are lists, they must have equal length." % caller)
    # invariant: from_arg and to_arg have equal length, or (at least) one has length 1

    if len(from_arg) == 1:
        from_arg = from_arg*len(to_arg)  # this is a no-op if len(to_arg)==1
    if len(to_arg) == 1:
        to_arg   = to_arg*len(from_arg)  # this is a no-op if len(from_arg)==1
    # invariant: from_arg and to_arg have equal length

    return from_arg, to_arg
    

def Displacement(from_arg, to_arg):
    """
    Obtain vector of lateral displacement from node(s) from_arg to node(s) to_arg.

    Parameters
    ----------
    from_arg  list of GIDs; or positions or single position (numpy.array(s))
    to_arg    list of GIDs

    Returns
    -------
    List of vectors of displacement between from and to.

    Displacement is always measured in the layer to which the "to_arg" node
    belongs. If a node in the "from_arg" list belongs to a different layer,
    its location is projected into the "to_arg" layer. If explicit positions
    are given in the "from_arg" list, they are interpreted in the "to_arg" layer.
    Displacement is the shortest displacement, taking into account
    periodic boundary conditions where applicable.

    If one of "from_arg" or "to_arg" has length 1, and the other is longer,
    the displacement from/to the single item to all other items is given.
    If "from_arg" and "to_arg" both have more than two elements, they have to be lists
    of the same length and the displacement for each pair is returned.

    See also
    --------
    Distance, nest.help("topology::Displacement")
    """

    from_arg, to_arg = _check_displacement_args(from_arg, to_arg, 'Displacement')
    return topology_func('{ Displacement } MapThread', [from_arg, to_arg])


def Distance(from_arg, to_arg):
    """
    Obtain vector of lateral distances from node(s) from_arg to node(s) to_arg.

    Parameters
    ----------
    from_arg  list of GIDs; or positions or single position (numpy.array(s))
    to_arg    list of GIDs

    Returns
    -------
    List of distances between from_arg and to_arg.

    Distance is always measured in the layer to which the "to_arg" node
    belongs. If a node in the "from_arg" list belongs to a different layer,
    its location is projected into the "to_arg" layer. If explicit positions
    are given in the "from_arg" list, they are interpreted in the "to_arg" layer.
    Distance is the shortest distance, taking into account
    periodic boundary conditions where applicable.

    If one of "from_arg" or "to_arg" has length 1, and the other is longer, the distance
    from/to the single item to all other items is given.
    If "from_arg" and "to_arg" both have more than two elements, they have to be lists
    of the same length and the distance of each pair is returned.

    See also
    --------
    Displacement, nest.help("topology::Distance")
    """

    from_arg, to_arg = _check_displacement_args(from_arg, to_arg, 'Distance')
    return topology_func('{ Distance } MapThread', [from_arg, to_arg])


def _rank_specific_filename(basename):
    """Returns file name decorated with rank."""
   
    if nest.NumProcesses() == 1:
        return basename
    else:
        np = nest.NumProcesses()
        np_digs = len(str(np-1))  # for pretty formatting
        rk = nest.Rank()
        dot = basename.find('.')
        if dot < 0:
            return '%s-%0*d' % (basename, np_digs, rk)
        else:
            return '%s-%0*d%s' % (basename[:dot], np_digs, rk, basename[dot:])

    
def DumpLayerNodes(layers, outname):
    """
    Write layer node positions to file.

    Parameters
    ----------
    layers   List of layer GIDs
    outname  Name of file to write to [will be overwritten if it exists]

    Write GID and position data to file. For each node in a layer, one line with 
    the following information is written:
    
        GID x-position y-position [z-position]
        
    If layers contains several GIDs, data for all layers will be written to a 
    single file.
    
    Note
    ----
    If calling this function from a distributed simulation, this function
    will write to one file per MPI rank. File names are formed by inserting
    the MPI Rank into the file name before the file name suffix. Each file
    stores data for nodes local to that file.

    See also
    --------
    DumpLayerConnections
    """
    topology_func("""
                  (w) file exch { DumpLayerNodes } forall close
                  """,
                  layers, _rank_specific_filename(outname))


def DumpLayerConnections(layers, synapse_model, outname):
    """
    Write connectivity information to file.

    Parameters
    ----------
    layers         List of layer GIDs
    synapse_model  Synapse model
    output         Name of file to write to

    This function writes connection information to file for all
    outgoing connections from the given layers with the given synapse model.
    Data for all layers in the list is combined.

    For each connection, one line is stored, in the following format:

    source_gid target_gid weight delay dx dy [dz]

    where (dx, dy [, dz]) is the displacement from source to target node.
    
    Note
    ----
    If calling this function from a distributed simulation, this function
    will write to one file per MPI rank. File names are formed by inserting
    the MPI Rank into the file name before the file name suffix. Each file
    stores data for connections local to that file.

    See also
    --------
    DumpLayerNodes
    """

    topology_func("""
                  /oname  Set 
                  cvlit /synmod Set
                  /lyrs   Set 
                  oname (w) file lyrs { synmod DumpLayerConnections } forall close  
                  """,
                  layers, synapse_model, _rank_specific_filename(outname))


def FindCenterElement(layers):
    """
    Return GID(s) of node closest to center of layer(s).

    Parameters
    ----------
    layers   List of layer GIDs

    Returns
    -------
    A list containing for each layer the GID of the node closest to the center
    of the layer, as specified in the layer parameters. If several nodes are
    equally close to the center, an arbitrary one of them is returned.

    See also
    --------
    FindNearestElement
    """ 
    
    nest.raise_if_not_list_of_gids(layers, 'layers')

    # we need to do each layer on its own since FindNearestElement does not thread
    return [FindNearestElement([lyr], nest.GetStatus([lyr], 'topology')[0]['center'])[0]
            for lyr in layers]


def GetTargetNodes(sources, tgt_layer, tgt_model=None, syn_model=None):
    """
    Obtain targets of a list of sources in a given target layer.
    
    Parameters
    ----------
    sources     List of GID(s) of source neurons
    tgt_layer   Single-element list with GID of tgt_layer
    tgt_model   Return only target positions for a given neuron model [optional].
    syn_model   Return only target positions for a given synapse model [optional].

    Returns
    -------
    List of GIDs of target neurons fulfilling the given criteria. It is a list of lists,
    one list per source.

    For each neuron in sources, this function finds all target elements in tgt_layer.
    If tgt_model is not given (default), all targets are returned, otherwise only
    targets of specific type, and similarly for syn_model.
    
    Note: For distributed simulations, this function only returns targets on the local MPI process.
    
    See also
    --------
    GetTargetPositions, nest.GetConnections
    """
    
    nest.raise_if_not_list_of_gids(sources, 'sources')
    nest.raise_if_not_list_of_gids(tgt_layer, 'tgt_layer')
    if len(tgt_layer) != 1:
        raise nest.NESTError("tgt_layer must be a one-element list")
    
    # obtain local nodes in target layer, to pass to GetConnections
    tgt_nodes = nest.GetLeaves(tgt_layer,
                               properties = {'model': tgt_model} if tgt_model else None,
                               local_only = True)[0]
                              
    conns = nest.GetConnections(sources, tgt_nodes, synapse_model=syn_model)
       
    # conns is a flat list of connections.
    # Re-organize into one list per source, containing only target GIDs.
    src_tgt_map = dict((sgid, []) for sgid in sources)
    for conn in conns:
       src_tgt_map[conn[0]].append(conn[1])

    # convert dict to nested list in same order as sources
    return [src_tgt_map[sgid] for sgid in sources]



def GetTargetPositions(sources, tgt_layer, tgt_model=None, syn_model=None):
    """
    Obtain positions of targets of a list of sources in a given target layer.
    
    Parameters
    ----------
    sources     List of GID(s) of source neurons
    tgt_layer   Single-element list with GID of tgt_layer
    tgt_model   Return only target positions for a given neuron model [optional].
    syn_type    Return only target positions for a given synapse model [optional].

    Returns
    -------
    Positions of target neurons fulfilling the given criteria as a nested list,
    containing one list of positions per node in sources.

    For each neuron in sources, this function finds all target elements in tgt_layer.
    If tgt_model is not given (default), all targets are returned, otherwise only
    targets of specific type, and similarly for syn_model.
    
    Note: For distributed simulations, this function only returns positions 
          of targets on the local MPI process.

    See also
    --------
    GetTargetNodes
    """
    
    return [GetPosition(nodes) for nodes
            in GetTargetNodes(sources, tgt_layer, tgt_model, syn_model)]
    

def _draw_extent(ax, xctr, yctr, xext, yext):
    """Draw extent and set aspect ration, limits"""

    import matplotlib.pyplot as plt

    # thin gray line indicating extent
    llx, lly = xctr - xext/2.0, yctr - yext/2.0
    urx, ury = llx + xext, lly + yext
    ax.add_patch(plt.Rectangle([llx, lly], xext, yext, fc='none', ec='0.5', lw=1, zorder=1))
    
    # set limits slightly outside extent
    ax.set(aspect='equal', 
           xlim=[llx - 0.05*xext, urx + 0.05*xext],
           ylim=[lly - 0.05*yext, ury + 0.05*yext],
           xticks=[], yticks=[])


def PlotLayer(layer, fig=None, nodecolor='b', nodesize=20):
    """
    Plot nodes in a layer.
    
    This function plots only top-level nodes, not the content of composite nodes.
    
    Note: You should not use this function in distributed simulations.
    
    Parameters
    ----------
    layer         GID of layer to plot (as single-element list)
    fig           Matplotlib figure to plot to. If not given, a new figure is created [optional].
    nodecolor     Color for nodes [optional].
    nodesize      Marker size for nodes [optional].

    Returns
    -------
    Matplotlib figure.
    
    See also
    --------
    PlotTargets
    """

    import matplotlib.pyplot as plt
    
    if len(layer) != 1:
        raise ValueError("layer must contain exactly one GID.")

    # get layer extent
    ext = nest.GetStatus(layer, 'topology')[0]['extent']

    if len(ext)==2:
        # 2D layer

        # get layer extent and center, x and y
        xext, yext = ext
        xctr, yctr = nest.GetStatus(layer, 'topology')[0]['center']
    
        # extract position information, transpose to list of x and y positions
        xpos, ypos = zip(*GetPosition(nest.GetChildren(layer)[0]))

        if not fig:
            fig = plt.figure()
            ax = fig.add_subplot(111)
        else:
            ax = fig.gca()

        ax.scatter(xpos, ypos, s=nodesize, facecolor=nodecolor, edgecolor='none')
        _draw_extent(ax, xctr, yctr, xext, yext)

    elif len(ext)==3:
        # 3D layer
        from mpl_toolkits.mplot3d import Axes3D

        # extract position information, transpose to list of x,y,z positions
        pos = zip(*GetPosition(nest.GetChildren(layer)[0]))

        if not fig:
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
    Plot all targets of src_nrn in a tgt_layer.
    
    Note: You should not use this function in distributed simulations.

    Parameters
    ----------
    src_nrn      GID of source neuron (as single-element list)
    tgt_layer    GID of tgt_layer (as single-element list)
    tgt_model    Show only targets of a given model [optional].
    syn_type     Show only targets connected to with a given synapse type [optional].
    fig          Matplotlib figure to plot to. If not given, new figure is created [optional].
    
    mask         Draw topology mask with targets; see PlotKernel for details [optional].
    kernel       Draw topology kernel with targets; see PlotKernel for details [optional].
    
    src_color    Color used to mark source node position [default: 'red']
    src_size     Size of source marker (see scatter for details) [default: 50]
    tgt_color    Color used to mark target node positions [default: 'blue']
    tgt_size     Size of target markers (see scatter for details) [default: 20]
    mask_color   Color used for line marking mask [default: 'red']
    kernel_color Color used for lines marking kernel [default: 'red']

    Returns
    -------
    Matplotlib figure.
    
    See also
    --------
    PlotLayer, GetTargetPositions
    matplotlib.pyplot.scatter
    """

    import matplotlib.pyplot as plt

    # get position of source
    srcpos = GetPosition(src_nrn)[0]

    # get layer extent and center, x and y
    ext = nest.GetStatus(tgt_layer, 'topology')[0]['extent']

    if len(ext)==2:
        # 2D layer

        # get layer extent and center, x and y
        xext, yext = ext
        xctr, yctr = nest.GetStatus(tgt_layer, 'topology')[0]['center']
    
        if not fig:
            fig = plt.figure()
            ax = fig.add_subplot(111)
        else:
            ax = fig.gca()

        # get positions, reorganize to x and y vectors
        tgtpos = GetTargetPositions(src_nrn, tgt_layer, tgt_model, syn_type)
        if tgtpos:
            xpos, ypos = zip(*tgtpos[0])
            ax.scatter(xpos, ypos, s=tgt_size, facecolor=tgt_color, edgecolor='none')

        ax.scatter(srcpos[:1], srcpos[1:], s=src_size, facecolor=src_color, edgecolor='none',
                   alpha = 0.4, zorder = -10)
    
        _draw_extent(ax, xctr, yctr, xext, yext)

        if mask or kernel:
            PlotKernel(ax, src_nrn, mask, kernel, mask_color, kernel_color)

    else:
        # 3D layer
        from mpl_toolkits.mplot3d import Axes3D

        if not fig:
            fig = plt.figure()
            ax = fig.add_subplot(111, projection='3d')
        else:
            ax = fig.gca()

        # get positions, reorganize to x,y,z vectors
        tgtpos = GetTargetPositions(src_nrn, tgt_layer, tgt_model, syn_type)
        if tgtpos:
            xpos, ypos, zpos = zip(*tgtpos[0])
            ax.scatter3D(xpos, ypos, zpos, s=tgt_size, facecolor=tgt_color, edgecolor='none')

        ax.scatter3D(srcpos[:1], srcpos[1:2], srcpos[2:], s=src_size, facecolor=src_color, edgecolor='none',
                   alpha = 0.4, zorder = -10)

    plt.draw_if_interactive() 

    return fig


def PlotKernel(ax, src_nrn, mask, kern=None, mask_color='red', kernel_color='red'):
    """
    Add indication of mask and kernel to axes.

    Adds solid red line for mask. For doughnut mask show inner and outer line. 
    If kern is Gaussian, add blue dashed lines marking 1, 2, 3 sigma.
    This function ignores periodic boundary conditions.
    Usually, this function is invoked by PlotTargets.

    Note: You should not use this function in distributed simulations.
    
    Parameters
    ----------
    ax        Axes returned by PlotTargets
    src_nrn   GID of source neuron  (as single element list), mask and kernel plotted relative to it.
    mask      Mask used in creating connections.    
    kern      Kernel used in creating connections.

    mask_color   Color used for line marking mask [default: 'red']
    kernel_color Color used for lines marking kernel [default: 'red']
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
        offs = np.array([0.,0.])

    if 'circular' in mask:
        r = mask['circular']['radius']
        ax.add_patch(plt.Circle(srcpos+offs, radius=r, zorder = -1000,
                                fc = 'none', ec = mask_color, lw=3))
    elif 'doughnut' in mask:
        r_in  = mask['doughnut']['inner_radius']
        r_out = mask['doughnut']['outer_radius']
        ax.add_patch(plt.Circle(srcpos+offs, radius=r_in, zorder = -1000,
                                fc = 'none', ec = mask_color, lw=3))
        ax.add_patch(plt.Circle(srcpos+offs, radius=r_out, zorder = -1000,
                                fc = 'none', ec = mask_color, lw=3))
    elif 'rectangular' in mask:
        ll = mask['rectangular']['lower_left']        
        ur = mask['rectangular']['upper_right']
        ax.add_patch(plt.Rectangle(srcpos+ll+offs, ur[0]-ll[0], ur[1]-ll[1],
                                   zorder=-1000, fc= 'none', ec=mask_color, lw=3))
    else:
        raise ValueError('Mask type cannot be plotted with this version of PyTopology.')

    if kern and isinstance(kern, dict):
        if 'gaussian' in kern:
            sigma = kern['gaussian']['sigma']
            for r in xrange(3):
                ax.add_patch(plt.Circle(srcpos+offs, radius=(r+1)*sigma, zorder=-1000,
                                        fc='none', ec=kernel_color, lw=3, ls='dashed'))
        else:
            raise ValueError('Kernel type cannot be plotted with this version of PyTopology')
                                                
    plt.draw()
    
