#! /usr/bin/env python
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
High-level API of PyNEST.

This file defines the user-level functions of NEST's Python interface
by mapping NEST/SLI commands to Python. Please try to follow these
rules:

1. SLI commands have the same name in Python. This means that most
   function names are written in camel case, although the Python
   guidelines suggest to use lower case for funtcion names. However,
   this way, it is easier for users to migrate from SLI to Python and.

2. Nodes are identified by their global IDs (GID) by default.

3. GIDs are always written as lists, e.g. [0], [1,2]

4. Commands that return a GID must return it as list of GID(s).

5. Commands that expect or return node addresses must indicate this in
   their name, e.g. GetAddress(list) expects a list of GIDs and
   returns a list of addresses.

6. Like GIDs, addresses must also appear only in lists. Thus,
   addresses and GIDs can be easily disambiguated:
   
   List of GIDs: [1,2,3,4]
   List of addresses: [[1,2],[3,4]]

7. When possible, loops over nodes should be propagated down to the
   SLI level.  This minimizes the number of Python<->SLI conversions
   and increases performance.  Loops in SLI are also faster than in
   Python. Example:

   instead of
   
   GetAddress(nodes):
     GetOneAddress(n):
        slipush(n)
        slirun("GetAddress")
     return map(GetOneAddress,nodes)

   write

   GetAddress(nodes):
     slipush(nodes)
     slirun("{GetAddress} map")
     return slipop()
     
8. If you have a good reason, you may deviate from these guidelines.

Authors: Jochen Eppler, Marc-Oliver Gewaltig, Moritz Helias, Eilif Mueller
"""

import string
import types

# These variables MUST be set by __init__.py right after importing.
# There is no safety net, whatsoever.
nest = sps = spp = sr = None

class NESTError(Exception):
    def __init__(self, msg) :
        Exception.__init__(self, msg)


# -------------------- Helper functions


def is_sequencetype(seq) :
    """Return True if the given object is a sequence type, False else"""
    
    return type(seq) in (types.TupleType, types.ListType)

def is_iterabletype(seq) :
    """Return True if the given object is iterable, False else"""

    try:
        i = iter(seq)
    except TypeError:
        return False

    return True

def is_sequence_of_nonneg_ints(seq):
    """Return True if the given object is a list or tuple of ints, False else"""
    return is_sequencetype(seq) and all([type(n) == type(0) and n >= 0 for n in seq])

def raise_if_not_list_of_gids(seq, argname):
    """
    Raise a NestError if seq is not a sequence of ints, otherwise, do nothing.
    The main purpose of this function is to perform a simple check that an
    argument is a potentially valid list of GIDs (ints >= 0).
    """
    if not is_sequence_of_nonneg_ints(seq):
        raise NESTError(argname + " must be a list or tuple of GIDs")
 
def broadcast(val, l, allowedtypes, name="val"):

    if type(val) in allowedtypes:
        return l*(val,)
    elif len(val)==1:
        return l*val
    elif len(val)!=l:
        raise NESTError("'%s' must be a single value, a list with one element or a list with %i elements." % (name, l))

    return val


def flatten(x):
    """flatten(sequence) -> list

    Returns a flat list with all elements from the sequence and all
    contained sub-sequences (iterables).

    Examples:
    >>> [1, 2, [3,4], (5,6)]
    [1, 2, [3, 4], (5, 6)]
    >>> flatten([[[1,2,3], (42,None)], [4,5], [6], 7, MyVector(8,9,10)])
    [1, 2, 3, 42, None, 4, 5, 6, 7, 8, 9, 10]"""

    result = []
    for el in x:
        if hasattr(el, "__iter__") and not isinstance(el, basestring) and not type(el) == types.DictType:
            result.extend(flatten(el))
        else:
            result.append(el)

    return result


# -------------------- Functions to get information on NEST


def sysinfo():
    """Print information on the platform on which NEST was compiled."""

    sr("sysinfo")


def version():
    """Print the NEST version."""

    sr("statusdict [[ /kernelname /version ]] get")
    return string.join(spp())
    

def authors():
    """Show the authors of NEST."""

    sr("authors")


def helpdesk(browser="firefox"):
    """Open the NEST helpdesk in the given browser."""
    
    sr("/helpdesk << /command (%s) >> SetOptions" % browser)
    sr("helpdesk")


def help(obj=None, pager="less"):
    """Show the help page for the given object"""

    if obj:
        sr("/page << /command (%s) >> SetOptions" % pager)
        sr("/%s help" % obj)
    else:
	print "Type 'nest.helpdesk()' to access the online documentation in a browser."
	print "Type 'nest.help(object)' to get help on a NEST object or command."
        print 
        print "Type 'nest.Models()' to see a list of available models in NEST."
        print 
	print "Type 'nest.authors()' for information about the makers of NEST."
	print "Type 'nest.sysinfo()' to see details on the system configuration."
	print "Type 'nest.version()' for information about the NEST version."
        print
	print "For more information visit http://www.nest-initiative.org."


# -------------------- Functions for simulation control


def Simulate(t):
    """Simulate the network for t milliseconds."""

    sps(float(t))
    sr('ms Simulate')


def ResumeSimulation():
    """Resume an interrupted simulation."""

    sr("ResumeSimulation")


def ResetKernel():
    """Reset the simulation kernel. This will destroy the network as
    well as all custom models created with CopyModel(). Calling this
    function is equivalent to restarting NEST."""

    sr('ResetKernel')


def ResetNetwork():
    """Reset all nodes and connections to their original state."""

    sr('ResetNetwork')


def SetKernelStatus(params):
    """Set parameters for the simulation kernel."""
    
    sps(0)
    sps(params)
    sr('SetStatus')


def GetKernelStatus(keys = None):
    """
    Obtain parameters of the simulation kernel.

    Returns:
    - Parameter dictionary if called without argument
    - Single parameter value if called with single parameter name
    - List of parameter values if called with list of parameter names
    """
    
    sr('0 GetStatus')
    rootstatus = spp()

    sr('/subnet GetDefaults')
    subnetdefaults = spp()

    subnetdefaults["address"] = None
    subnetdefaults["frozen"] = None
    subnetdefaults["global_id"] = None
    subnetdefaults["local"] = None
    subnetdefaults["local_id"] = None
    subnetdefaults["parent"] = None
    subnetdefaults["state"] = None
    subnetdefaults["thread"] = None
    subnetdefaults["vp"] = None

    d = dict()

    for k in rootstatus :
        if k not in subnetdefaults :
            d[k] = rootstatus[k]

    if not keys:
        return d
    elif is_sequencetype(keys):
        return [d[k] for k in keys]
    else:
        return d[keys]

def Install(module_name):
    """
    Load a dynamically linked NEST module.

    Example:
    nest.Install("mymodule")

    Returns:
    NEST module identifier, required for unloading.

    Note:
    Dynamically linked modules are search in the LD_LIBRARY_PATH
    (DYLD_LIBRARY_PATH under OSX). 
    """

    return sr("(%s) Install" % module_name)


# -------------------- Functions for parallel computing


def Rank():
    """Return the MPI rank of the local process."""

    sr("Rank")
    return spp()

def NumProcesses():
    """Return the overall number of MPI processes."""

    sr("NumProcesses")
    return spp()

def SetAcceptableLatency(port, latency):
    """Set the acceptable latency (in ms) for a MUSIC port."""
    
    sps(latency)
    sr("/%s exch SetAcceptableLatency" % port)


# -------------------- Functions for model handling


def Models(mtype = "all", sel=None):
    """Return a list of all available models (neurons, devices and
    synapses). Use mtype='nodes' to only see neuron and device models,
    mtype='synapses' to only see synapse models. sel can be a string,
    used to filter the result list and only return models containing
    it."""

    if mtype not in ("all", "nodes", "synapses"):
        raise NESTError("type has to be one of 'all', 'nodes' or 'synapses'.")

    models = list()

    if mtype in ("all", "nodes"):
       sr("modeldict")
       models += spp().keys()

    if mtype in ("all", "synapses"):
       sr("synapsedict")
       models += spp().keys()
    
    if sel != None:
        models = filter(lambda x: x.find(sel) >= 0, models)

    models.sort()
    return models


def SetDefaults(model, params) :
    """Set the default parameters of the given model to the values
    specified in the params dictionary."""

    sps(params)
    sr('/%s exch SetDefaults' % model)


def GetDefaults(model) :
    """Return a dictionary with the default parameters of the given
    model, specified by a string."""
    
    sr("/%s GetDefaults" % model)
    return spp()


def CopyModel(existing, new, params=None):
    """ Create a new model by copying an existing one. Default
    parameters can be given as params, or else are taken from
    existing."""
    
    if params:
        sps(params)
        sr("/%s /%s 3 2 roll CopyModel" % (existing, new))
    else:
        sr("/%s /%s CopyModel" % (existing, new))


# -------------------- Functions for node handling

def Create(model, n=1, params=None):
    """Create n instances of type model. Parameters for the new nodes
    can are given as params (a single dictionary or a list of
    dictionaries with size n). If omitted, the model's defaults are
    used."""

    broadcast_params = False

    sps(n)
    cmd = "/%s exch Create" % model

    if params:
        if type(params) == types.DictType:
            sps(params)
            cmd = "/%s 3 1 roll Create" % model
        elif is_sequencetype(params) and (len(params) == 1 or len(params) == n):
            broadcast_params = True
        else:
            sr(";") # pop n from the stack
            raise NESTError("params has to be a single dictionary or a list of dictionaries with size n.")
        
    sr(cmd)

    lastgid = spp()
    ids = range(lastgid - n + 1, lastgid + 1)

    if broadcast_params:
        try:
            SetStatus(ids, broadcast(params, n, (dict,)))
        except:
            raise NESTError("SetStatus failed, but nodes already have been created. The ids of the new nodes are: %s" % ids)

    return ids

        
def SetStatus(nodes, params, val=None) :
    """ Set the parameters of nodes (identified by global ids or
    addresses) or connections (identified by handles as returned by
    FindConnections()) to params, which may be a single dictionary or
    a list of dictionaries. If val is given, params has to be the name
    of an attribute, which is set to val on the nodes/connections. val
    can be a single value or a list of the same size as nodes."""

    if not is_sequencetype(nodes):
        raise NESTError("nodes must be a list of nodes or synapses.")

    if len(nodes) == 0:
        return

    if type(params) == types.StringType :
        if is_iterabletype(val) and not type(val) in (types.StringType, types.DictType):
            params = [{params : x} for x in val]
        else :
            params = {params : val}

    params = broadcast(params, len(nodes), (dict,), "params")
    if len(nodes) != len(params) :
        raise NESTError("Status dict must be a dict, or list of dicts of length 1 or len(nodes).")
        
    if type(nodes[0]) == types.DictType:
        nest.push_connection_datums(nodes)
    else:
        sps(nodes)

    sps(params)
    sr('2 arraystore')
    sr('Transpose { arrayload ; SetStatus } forall')

def GetStatus(nodes, keys=None) :
    """Return the parameter dictionaries of the given list of nodes
    (identified by global ids or addresses) or connections (identified
    by handles as returned by FindConnections()). If keys is given, a
    list of values is returned instead. keys may also be a list, in
    which case the returned list contains lists of values."""

    if not is_sequencetype(nodes):
        raise NESTError("nodes must be a list of nodes or synapses.")

    if len(nodes) == 0:
        return nodes

    cmd='{ GetStatus } Map'

    if keys:
        if is_sequencetype(keys):
            keyss = string.join(["/%s" % x for x in keys])
            cmd='{ GetStatus } Map { [ [ %s ] ] get } Map' % keyss
        else:
            cmd='{ GetStatus /%s get} Map' % keys

    if type(nodes[0]) == types.DictType:
        nest.push_connection_datums(nodes)
    else:
        sps(nodes)
   
    sr(cmd)

    return spp()


def GetAddress(gids) :
    """Return the addresses of one or more nodes."""

    sps(gids)
    sr("{GetAddress} Map")
    return spp()


def GetGID(addresses) :
    """Return the global ids of one or more nodes."""

    sps(addresses)
    sr("{GetGID} Map")
    return spp()


def GetLID(gid) :
    """
    Return the local id of a node with gid.
    GetLID(gid) -> lid
    Example:
      gid=GetGID([[1,2,n]])
      GetLID(gid) -> n
    """
    if len(gid) > 1:
        raise NESTError("Can only return the local ID of one node.")
    sps(gid[0])
    sr("GetLID")

    return spp()


# -------------------- Functions for connection handling
        

def FindConnections(source, target=None, synapse_type=None) :
    """Return an array of identifiers for connections that match the
    given parameters. Only source is mandatory and must be a list of
    one or more nodes. If target and/or synapse_type is/are given,
    they must be single values, lists of length one or the same length
    as source. Use GetStatus()/SetStatus() to inspect/modify the found
    connections."""

    if not target and not synapse_type:
        params = [{"source": s} for s in source]

    if not target and synapse_type:
        synapse_type = broadcast(synapse_type, len(source), (str,), "synapse_type")
        params = [{"source": s, "synapse_type": syn} for s, syn in zip(source, synapse_type)]

    if target and not synapse_type:
        target = broadcast(target, len(source), (int,), "target")
        params = [{"source": s, "target": t} for s, t in zip(source, target)]

    if target and synapse_type:
        target = broadcast(target, len(source), (int,), "target")
        synapse_type = broadcast(synapse_type, len(source), (str,), "synapse_type")
        params = [{"source": s, "target": t, "synapse_type": syn} for s, t, syn in zip(source, target, synapse_type)]

    sps(params)
    sr("{FindConnections} Map" % params)
    
    return flatten(spp())


def Connect(pre, post, params=None, delay=None, model="static_synapse"):
    """Make one-to-one connections of type model between the nodes in
    pre and the nodes in post. pre and post have to be lists of the
    same length. If params is given (as dictionary or list of
    dictionaries), they are used as parameters for the connections. If
    params is given as a single float or as list of floats, it is used
    as weight(s), in which case delay also has to be given as float or
    as list of floats."""

    if len(pre) != len(post):
        raise NESTError("pre and post have to be the same length")

    # pre post Connect
    if params == None and delay == None:
        for s,d in zip(pre, post):
            sps(s)
            sps(d)
            sr('/%s Connect' % model)

    # pre post params Connect
    elif params != None and delay == None:
        params = broadcast(params, len(pre), (dict,), "params")
        if len(params) != len(pre):
            raise NESTError("params must be a dict, or list of dicts of length 1 or len(pre).")

        for s,d,p in zip(pre, post, params) :
            sps(s)
            sps(d)
            sps(p)
            sr('/%s Connect' % model)

    # pre post w d Connect
    elif params != None and delay != None:
        params = broadcast(params, len(pre), (float,), "params")
        if len(params) != len(pre):
            raise NESTError("params must be a float, or list of floats of length 1 or len(pre) and will be used as weight(s).")
        delay = broadcast(delay, len(pre), (float,), "delay")
        if len(delay) != len(pre):
            raise NESTError("delay must be a float, or list of floats of length 1 or len(pre).")

        for s,d,w,dl in zip(pre, post, params, delay) :
            sps(s)
            sps(d)
            sps(w)
            sps(dl)
            sr('/%s Connect' % model)

    else:
        raise NESTError("Both 'params' and 'delay' have to be given.")


def ConvergentConnect(pre, post, weight=None, delay=None, model="static_synapse"):
    """Connect all neurons in pre to each neuron in post. pre and post
    have to be lists. If weight is given (as a single float or as list
    of floats), delay also has to be given as float or as list of
    floats."""

    if weight == None and delay == None:
        for d in post :
            sps(pre)
            sps(d)
            sr('/%s ConvergentConnect' % model)

    elif weight != None and delay != None:
        weight = broadcast(weight, len(pre), (float,), "weight")
        if len(weight) != len(pre):
            raise NESTError("weight must be a float, or sequence of floats of length 1 or len(pre)")
        delay = broadcast(delay, len(pre), (float,), "delay")
        if len(delay) != len(pre):
            raise NESTError("delay must be a float, or sequence of floats of length 1 or len(pre)")
        
        for d in post:
            sps(pre)
            sps(d)
            sps(weight)
            sps(delay)
            sr('/%s ConvergentConnect' % model)

    else:
        raise NESTError("Both 'weight' and 'delay' have to be given.")


def RandomConvergentConnect(pre, post, n, weight=None, delay=None,
                            model="static_synapse", options=None):
    
    """Connect n randomly selected neurons from pre to each neuron in
    post. pre and post have to be lists. If weight is given (as a
    single float or as list of floats), delay also has to be given as
    float or as list of floats. options is a dictionary specifying
    options to the RandomConvergentConnect function: allow_autapses,
    allow_multapses.
    """

    # store current options, set desired options
    old_options = None
    error = False
    if options:
        old_options = sli_func('GetOptions', '/RandomConvergentConnect',
                               litconv=True)
        del old_options['DefaultOptions'] # in the way when restoring
        sli_func('SetOptions', '/RandomConvergentConnect', options,
                 litconv=True)

    if weight == None and delay == None:
        sli_func(
            '/m Set /n Set /pre Set { pre exch n m RandomConvergentConnect } forall',
            post, pre, n, '/'+model, litconv=True)
    
    elif weight != None and delay != None:
        weight = broadcast(weight, n, (float,), "weight")
        if len(weight) != n:
            raise NESTError("weight must be a float, or sequence of floats of length 1 or n")
        delay = broadcast(delay, n, (float,), "delay")
        if len(delay) != n:
            raise NESTError("delay must be a float, or sequence of floats of length 1 or n")

        sli_func(
            '/m Set /d Set /w Set /n Set /pre Set { pre exch n w d m RandomConvergentConnect } forall',
            post, pre, n, weight, delay, '/'+model, litconv=True)
    
    else:
        error = True

    # restore old options
    if old_options:
        sli_func('SetOptions', '/RandomConvergentConnect', old_options,
                 litconv=True)

    if error:
        raise NESTError("Both 'weight' and 'delay' have to be given.")


def DivergentConnect(pre, post, weight=None, delay=None, model="static_synapse"):
    """Connect each neuron in pre to all neurons in post. pre and post
    have to be lists. If weight is given (as a single float or as list
    of floats), delay also has to be given as float or as list of
    floats."""

    if weight == None and delay == None:
        for s in pre :
            sps(s)
            sps(post)
            sr('/%s DivergentConnect' % model)

    elif weight != None and delay != None:
        weight = broadcast(weight, len(post), (float,), "weight")
        if len(weight) != len(post):
            raise NESTError("weight must be a float, or sequence of floats of length 1 or len(post)")
        delay = broadcast(delay, len(post), (float,), "delay")
        if len(delay) != len(post):
            raise NESTError("delay must be a float, or sequence of floats of length 1 or len(post)")

        for s in pre :
            sps(s)
            sps(post)
            sps(weight)
            sps(delay)
            sr('/%s DivergentConnect' % model)
    
    else:
        raise NESTError("Both 'weight' and 'delay' have to be given.")


def RandomDivergentConnect(pre, post, n, weight=None, delay=None,
                           model="static_synapse", options=None):
    """Connect each neuron in pre to n randomly selected neurons from
    post. pre and post have to be lists. If weight is given (as a
    single float or as list of floats), delay also has to be given as
    float or as list of floats. options is a dictionary specifying
    options to the RandomDivergentConnect function: allow_autapses,
    allow_multapses.
    """
    
    # store current options, set desired options
    old_options = None
    error = False
    if options:
        old_options = sli_func('GetOptions', '/RandomDivergentConnect',
                               litconv=True)
        del old_options['DefaultOptions'] # in the way when restoring
        sli_func('SetOptions', '/RandomDivergentConnect', options,
                 litconv=True)

    if weight == None and delay == None:
        sli_func(
            '/m Set /n Set /post Set { n post m RandomDivergentConnect } forall',
            pre, post, n, '/'+model, litconv=True)

    elif weight != None and delay != None:
        weight = broadcast(weight, n, (float,), "weight")
        if len(weight) != n:
            raise NESTError("weight must be a float, or sequence of floats of length 1 or n")
        delay = broadcast(delay, n, (float,), "delay")
        if len(delay) != n:
            raise NESTError("delay must be a float, or sequence of floats of length 1 or n")

        sli_func(
            '/m Set /d Set /w Set /n Set /post Set { n post w d m RandomDivergentConnect } forall',
            pre, post, n, weight, delay, '/'+model, litconv=True)

    else:
        error = True

    # restore old options
    if old_options:
        sli_func('SetOptions', '/RandomDivergentConnect', old_options,
                 litconv=True)

    if error:
        raise NESTError("Both 'weight' and 'delay' have to be given.")



# -------------------- Functions for hierarchical networks


def PrintNetwork(depth=1, subnet=None) :
    """Print the network tree up to depth, starting at subnet. if
    subnet is omitted, the current subnet is used instead."""
    
    if subnet == None:
        subnet = CurrentSubnet()

    sps(subnet[0])
    sr("GetAddress %i PrintNetwork" % depth)


def CurrentSubnet() :
    """Returns the global id of the current subnet."""

    sr("CurrentSubnet GetGID")
    return [spp()]


def ChangeSubnet(subnet) :
    """Make subnet the current subnet."""
    subnet = GetAddress(subnet)
    sps(subnet[0])
    sr("ChangeSubnet")


def GetLeaves(subnets) :
    """Return the global ids of all leaf nodes of the given
    subnet."""

    sps(subnets)
    sr("{GetLeaves} Map")
    return spp()


def GetNodes(subnets):
    """Return the complete list of nodes (including sub-networks) under
    subnet."""

    sps(subnets)
    sr("{GetNodes} Map")
    
    return spp()


def GetChildren(gid):
    """
    Return the immediate children of subnet id.
    """
    if len(gid)>1 :
        raise NESTError("GetChildren() expects exactly one GID.")
    sps(gid[0])
    sr("GetChildren")
    return spp()

        
def GetNetwork(gid, depth):
    """Return a nested list with the children of subnet id at level
    depth. If depth==0, the immediate children of the subnet are
    returned. The returned list is depth+1 dimensional.
    """
    
    if len(gid)>1 :
        raise NESTError("GetNetwork() expects exactly one GID.")
    
    sps(gid[0])
    sps(depth)
    sr("GetNetwork")
    return spp()


def GetNodeAt(gidlist,adr):
    """
    For each subnet in gidlist, get the gid of the node at position adr.
    """
    def get_node(subnet, adr):
        if type(subnet) == types.IntType:
            subnet= GetAddress([subnet])
        return GetGID([subnet[0]+adr])[0]
    
    return map(lambda n: get_node(n,adr),flatten(gidlist))


def BeginSubnet(label=None, params=None):
    """Create a new subnet and change into it.
    A string argument can be used to name the new subnet
    A dictionary argument can be used to set the subnet's custom dict."""

    sn=Create("subnet")
    if label:
        SetStatus(sn, "label", label)
    if params:
        SetStatus(sn, "customdict", params)
    ChangeSubnet(sn)


def EndSubnet():
    """Change to the parent subnet and return the gid of the current."""

    csn=CurrentSubnet()
    parent=GetStatus(csn, "parent")

    if csn != parent:
        ChangeSubnet(parent)
        return csn
    else:
        raise NESTError("Unexpected EndSubnet(). Cannot go higher than the root node.")


def LayoutNetwork(model, dim, label=None, params=None) :
    """Create a subnetwork of dimension dim with nodes of type model
       and return a list of ids."""
    if type(model) == types.StringType:
        sps(dim)
        sr('/%s exch LayoutNetwork' % model)
        if label:
            sr("dup << /label (%s) >> SetStatus"%label)
        if params:
            sr("dup << /customdict")
            sps(params)
            sr(">> SetStatus")
        return [spp()]

    # If model is a function.
    elif type(model) == types.FunctionType:
        # The following code uses a model constructor function
        # model() instead of a model name string.
        BeginSubnet(label, params)

        if len(dim)==1:
            for i in xrange(dim[0]):
                model()
        else:
            for i in xrange(dim[0]):
                LayoutNetwork(model,dim[1:])

        gid = EndSubnet()
        return gid

    else:
        raise NESTError("model must be a string or a function.")
