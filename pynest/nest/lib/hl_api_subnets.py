# -*- coding: utf-8 -*-
#
# hl_api_subnets.py
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
Functions for hierarchical networks
"""

from .hl_api_helper import *
from .hl_api_nodes import Create
from .hl_api_info import GetStatus, SetStatus


@check_stack
def PrintNodes():
    """Print nodes in the network.

    PrintNodes will replace PrintNetwork in NEST 3.0, so this function is here
    to allow users to switch now if they want to.
    """

    with SuppressedDeprecationWarning('PrintNetwork'):
        PrintNetwork()


@check_stack
@deprecated('', 'PrintNetwork is deprecated and will be removed in NEST 3.0. \
Use PrintNodes instead.')
def PrintNetwork(depth=1, subnet=None):
    """Print the network tree up to depth, starting at subnet.

    If subnet is omitted, the current subnet is used instead.

    Parameters
    ----------
    depth : int, optional
        Depth to print to
    subnet : TYPE, optional
        Subnet to start at

    Raises
    ------
    NESTError
    """

    if subnet is None:
        # Avoid confusing user by deprecation warning
        with SuppressedDeprecationWarning('CurrentSubnet'):
            subnet = CurrentSubnet()
    elif len(subnet) > 1:
        raise NESTError("PrintNetwork() expects exactly one GID.")

    sps(subnet[0])
    sr("%i PrintNetwork" % depth)


@check_stack
@deprecated('', 'CurrentSubnet is deprecated and will be removed in NEST 3.0.')
def CurrentSubnet():
    """Returns the global id of the current subnet.

    Returns
    -------
    int:
        GID of current subnet
    """

    sr("CurrentSubnet")
    return (spp(), )


@check_stack
@deprecated('', 'ChangeSubnet is deprecated and will be removed in NEST 3.0.')
def ChangeSubnet(subnet):
    """Make given subnet the current.

    Parameters
    ----------
    subnet : int
        GID of the subnet

    Raises
    ------
    NESTError
    """

    if len(subnet) > 1:
        raise NESTError("ChangeSubnet() expects exactly one GID.")

    sps(subnet[0])
    sr("ChangeSubnet")


@check_stack
@deprecated('', 'GetLeaves is deprecated and will be removed in NEST 3.0. Use \
GIDCollection instead.')
def GetLeaves(subnets, properties=None, local_only=False):
    """Return the GIDs of the leaf nodes of the given subnets.

    Leaf nodes are all nodes that are not subnets.

    Parameters
    ----------
    subnets : list
        GIDs of subnets
    properties : dict, optional
        Only global ids of nodes matching the properties given in the
        dictionary exactly will be returned. Matching properties with float
        values (e.g. the membrane potential) may fail due to tiny numerical
        discrepancies and should be avoided.
    local_only : bool, optional
        If True, only GIDs of nodes simulated on the local MPI process will
        be returned. By default, global ids of nodes in the entire simulation
        will be returned. This requires MPI communication and may slow down
        the script.

    Returns
    -------
    list:
        GIDs of leaf nodes

    See also
    --------
    GetNodes
    GetChildren
    """

    if properties is None:
        properties = {}
    func = 'GetLocalLeaves' if local_only else 'GetGlobalLeaves'
    return sli_func('/props Set { props %s } Map' % func, subnets, properties,
                    litconv=True)


@check_stack
@deprecated('', 'GetNodes is deprecated and will be removed in NEST 3.0. Use \
GIDCollection instead.')
def GetNodes(subnets, properties=None, local_only=False):
    """Return the global ids of the all nodes of the given subnets.

    Parameters
    ----------
    subnets : list
        GIDs of subnets
    properties : dict, optional
        Only global ids of nodes matching the properties given in the
        dictionary exactly will be returned. Matching properties with float
        values (e.g. the membrane potential) may fail due to tiny numerical
        discrepancies and should be avoided.
    local_only : bool, optional
        If True, only GIDs of nodes simulated on the local MPI process will
        be returned. By default, global ids of nodes in the entire simulation
        will be returned. This requires MPI communication and may slow down
        the script.

    Returns
    -------
    list:
        GIDs of leaf nodes

    See also
    --------
    GetLeaves
    GetChildren
    """

    if properties is None:
        properties = {}
    func = 'GetLocalNodes' if local_only else 'GetGlobalNodes'
    return sli_func('/props Set { props %s } Map' % func, subnets, properties,
                    litconv=True)


@check_stack
@deprecated('',
            'GetChilden is deprecated and will be removed in NEST 3.0. Use GIDCollection instead.')  # noqa
def GetChildren(subnets, properties=None, local_only=False):
    """Return the global ids of the immediate children of the given subnets.

    Parameters
    ----------
    subnets : list
        GIDs of subnets
    properties : dict, optional
        Only global ids of nodes matching the properties given in the
        dictionary exactly will be returned. Matching properties with float
        values (e.g. the membrane potential) may fail due to tiny numerical
        discrepancies and should be avoided.
    local_only : bool, optional
        If True, only GIDs of nodes simulated on the local MPI process will
        be returned. By default, global ids of nodes in the entire simulation
        will be returned. This requires MPI communication and may slow down
        the script.

    Returns
    -------
    list:
        GIDs of leaf nodes

    See also
    --------
    GetLeaves
    GetNodes
    """

    if properties is None:
        properties = {}
    func = 'GetLocalChildren' if local_only else 'GetGlobalChildren'
    return sli_func('/props Set { props %s } Map' % func, subnets, properties,
                    litconv=True)


@check_stack
@deprecated('', 'GetNetwork is deprecated and will be removed in Nest 3.0.\
Script is responsible for retaining structure information if needed')
def GetNetwork(gid, depth):
    """Return a nested list with the children of subnet id at level
    depth.

    Parameters
    ----------
    gid : int
        GID of subnet
    depth : int
        Depth of list to return. If depth==0, the immediate children of the
        subnet are returned. The returned list is depth+1 dimensional.

    Returns
    -------
    list:
        nested lists of GIDs of child nodes

    Raises
    ------
    NESTError
    """

    if len(gid) > 1:
        raise NESTError("GetNetwork() expects exactly one GID.")

    sps(gid[0])
    sps(depth)
    sr("GetNetwork")
    return spp()


@check_stack
@deprecated('', 'BeginSubnet is deprecated and will be removed in NEST 3.0. \
Use GIDCollection instead.')
def BeginSubnet(label=None, params=None):
    """Create a new subnet and change into it.

    Parameters
    ----------
    label : str, optional
        Name of the new subnet
    params : dict, optional
        The customdict of the new subnet
    """

    sn = Create("subnet")
    if label is not None:
        SetStatus(sn, "label", label)
    if params is not None:
        SetStatus(sn, "customdict", params)
    ChangeSubnet(sn)


@check_stack
@deprecated('', 'EndSubnet is deprecated and will be removed in NEST 3.0. Use \
GIDCollection instead.')
def EndSubnet():
    """Change to the parent subnet and return the gid of the current.

    Raises
    ------
    NESTError
        Description
    """

    csn = CurrentSubnet()
    parent = GetStatus(csn, "parent")

    if csn != parent:
        ChangeSubnet(parent)
        return csn
    else:
        raise NESTError(
            "Unexpected EndSubnet(). Cannot go higher than the root node.")


@check_stack
@deprecated('', 'LayoutNetwork is deprecated and will be removed in NEST 3.0. \
Use Create(<model>, n=<number>) instead.')
def LayoutNetwork(model, dim, label=None, params=None):
    """Create a subnetwork of dimension dim with nodes of type model and
    return a list of ids.

    params is a dictionary, which will be set as
    customdict of the newly created subnet.

    Parameters
    ----------
    model : str
        Neuron model to use
    dim : int
        Dimension of subnetwork
    label : str, optional
        Name of the new subnet
    params : dict, optional
        The customdict of the new subnet. Not the parameters
        for the neurons in the subnetwork.

    Raises
    ------
    ValueError
        Description
    """

    if is_literal(model):
        sps(dim)
        sr('/%s exch LayoutNetwork' % model)
        if label is not None:
            sr("dup << /label (%s) >> SetStatus" % label)
        if params is not None:
            sr("dup << /customdict")
            sps(params)
            sr(">> SetStatus")
        return (spp(), )
    elif inspect.isfunction(model):
        BeginSubnet(label, params)
        if len(dim) == 1:
            [model() for _ in range(dim[0])]
        else:
            [LayoutNetwork(model, dim[1:]) for _ in range(dim[0])]
        gid = EndSubnet()
        return gid
    else:
        raise ValueError("model must be a string or a function")
