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

5. When possible, loops over nodes should be propagated down to the
   SLI level.  This minimizes the number of Python<->SLI conversions
   and increases performance.  Loops in SLI are also faster than in
   Python. 
        
6. If you have a *very* good reason, you may deviate from these guidelines.

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

def is_ndarray(seq):
        try:
                import numpy
                return type(seq) == numpy.ndarray
        except:
                return False
        
def is_sequencetype(seq) :
    """
    Return True if the given object is a sequence type, False else
    """
    import sys
    
    return (type(seq) in (types.TupleType, types.ListType)) or is_ndarray(seq)


def is_iterabletype(seq) :
    """
    Return True if the given object is iterable, False else
    """

    try:
        i = iter(seq)
    except TypeError:
        return False

    return True


def is_sequence_of_nonneg_ints(seq):
    """
    Return True if the given object is a list or tuple of ints, False else
    """

    return is_sequencetype(seq) and all([isinstance(n,int) and n >= 0 for n in seq])


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
    """
    flatten(sequence) -> list

    Returns a flat list with all elements from the sequence and all
    contained sub-sequences (iterables).

    Examples:
    >>> [1, 2, [3,4], (5,6)]
    [1, 2, [3, 4], (5, 6)]
    >>> flatten([[[1,2,3], (42,None)], [4,5], [6], 7, MyVector(8,9,10)])
    [1, 2, 3, 42, None, 4, 5, 6, 7, 8, 9, 10]
    """

    result = []
    for el in x:
        if hasattr(el, "__iter__") and not isinstance(el, basestring) and not type(el) == types.DictType:
            result.extend(flatten(el))
        else:
            result.append(el)

    return result


# -------------------- Functions to get information on NEST

def sysinfo():
    """
    Print information on the platform on which NEST was compiled.
    """

    sr("sysinfo")


def version():
    """
    Return the NEST version.
    """

    sr("statusdict [[ /kernelname /version ]] get")
    return string.join(spp())
    

def authors():
    """
    Print the authors of NEST.
    """

    sr("authors")


def helpdesk(browser="firefox"):
    """
    Open the NEST helpdesk in the given browser. The default browser is firefox.
    """
    
    sr("/helpdesk << /command (%s) >> SetOptions" % browser)
    sr("helpdesk")


def help(obj=None, pager="less"):
    """
    Show the help page for the given object using the given pager. The
    default pager is less.
    """

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


def get_verbosity():
    """
    Return verbosity level of NEST's messages.
    """
    
    sr('verbosity')
    return spp()


def set_verbosity(level):
    """
    Change verbosity level for NEST's messages. level is a string and
    can be one of M_FATAL, M_ERROR, M_WARNING, or M_INFO.
    """

    sr("%s setverbosity" % level)


def message(level,sender,text):
    """
    Print a message using NEST's message system.
    """

    sps(level)
    sps(sender)
    sps(text)
    sr('message')


# -------------------- Functions for simulation control

def Simulate(t):
    """
    Simulate the network for t milliseconds.
    """

    sps(float(t))
    sr('ms Simulate')


def ResumeSimulation():
    """
    Resume an interrupted simulation.
    """

    sr("ResumeSimulation")


def ResetKernel():
    """
    Reset the simulation kernel. This will destroy the network as
    well as all custom models created with CopyModel(). Calling this
    function is equivalent to restarting NEST.
    """

    sr('ResetKernel')


def ResetNetwork():
    """
    Reset all nodes and connections to their original state.
    """

    sr('ResetNetwork')


def SetKernelStatus(params):
    """
    Set parameters for the simulation kernel.
    """
    
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

    Note: Dynamically linked modules are searched in the
    LD_LIBRARY_PATH (DYLD_LIBRARY_PATH under OSX).
    """

    return sr("(%s) Install" % module_name)


# -------------------- Functions for parallel computing

def Rank():
    """
    Return the MPI rank of the local process.
    """

    sr("Rank")
    return spp()

def NumProcesses():
    """
    Return the overall number of MPI processes.
    """

    sr("NumProcesses")
    return spp()

def SetAcceptableLatency(port, latency):
    """
    Set the acceptable latency (in ms) for a MUSIC port.
    """
    
    sps(latency)
    sr("/%s exch SetAcceptableLatency" % port)


# -------------------- Functions for model handling

def Models(mtype = "all", sel=None):
    """
    Return a list of all available models (neurons, devices and
    synapses). Use mtype='nodes' to only see neuron and device models,
    mtype='synapses' to only see synapse models. sel can be a string,
    used to filter the result list and only return models containing
    it.
    """

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


def SetDefaults(model, params, val=None) :
    """
    Set the default parameters of the given model to the values
    specified in the params dictionary.
    If val is given, params has to be the name of a model property.
    New default values are used for all subsequently created instances
    of the model.
    """

    if type(params) == types.StringType :
            params = {params : val}

    sr('/'+model)
    sps(params)
    sr('SetDefaults')


def GetDefaults(model, keys=None) :
    """
    Return a dictionary with the default parameters of the given
    model, specified by a string.
    If keys is given, it must be a string or a list of strings naming model properties.
    GetDefaults then returns a single value or a list of values belonging to the keys given.
    Examples:
    GetDefaults('iaf_neuron','V_m') -> -70.0
    GetDefaults('iaf_neuron',['V_m', 'model') -> [-70.0, 'iaf_neuron']
    """
    cmd = "/%s GetDefaults" % model
    if keys:
        if is_sequencetype(keys):
            keyss = string.join(["/%s" % x for x in keys])
            cmd='/'+model+' GetDefaults  [ %s ] { 1 index exch get} Map' % keyss
        else:
            cmd= '/'+model+' GetDefaults '+'/'+keys+' get'
        
    sr(cmd)
    return spp()


def CopyModel(existing, new, params=None):
    """
    Create a new model by copying an existing one. Default parameters
    can be given as params, or else are taken from existing.
    """
    
    if params:
        sps(params)
        sr("/%s /%s 3 2 roll CopyModel" % (existing, new))
    else:
        sr("/%s /%s CopyModel" % (existing, new))


# -------------------- Functions for node handling

def Create(model, n=1, params=None):
    """
    Create n instances of type model. Parameters for the new nodes can
    are given as params (a single dictionary or a list of dictionaries
    with size n). If omitted, the model's defaults are used.
    """

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
    """
    Set the parameters of nodes (identified by global ids) or
    connections (identified by handles as returned by
    GetConnections()) to params, which may be a single dictionary or a
    list of dictionaries. If val is given, params has to be the name
    of an attribute, which is set to val on the nodes/connections. val
    can be a single value or a list of the same size as nodes.
    """

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
        
    if  (type(nodes[0]) == types.DictType) or is_sequencetype(nodes[0]):
        nest.push_connection_datums(nodes)
    else:
        sps(nodes)

    sps(params)
    sr('2 arraystore')
    sr('Transpose { arrayload ; SetStatus } forall')


def GetStatus(nodes, keys=None) :
    """
    Return the parameter dictionaries of the given list of nodes
    (identified by global ids) or connections (identified
    by handles as returned by GetConnections()). If keys is given, a
    list of values is returned instead. keys may also be a list, in
    which case the returned list contains lists of values.
    """

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

    if (type(nodes[0]) == types.DictType) or is_sequencetype(nodes[0]):
        nest.push_connection_datums(nodes)
    else:
        sps(nodes)
   
    sr(cmd)

    return spp()


def GetLID(gid) :
    """
    Return the local id of a node with gid.
    GetLID(gid) -> lid
    """

    if len(gid) > 1:
        raise NESTError("GetLID() expects exactly one GID.")

    sps(gid[0])
    sr("GetLID")

    return spp()


# -------------------- Functions for connection handling

	
def FindConnections(source, target=None, synapse_model=None, synapse_type=None):
    """
    Return an array of identifiers for connections that match the
    given parameters. Only source is mandatory and must be a list of
    one or more nodes. If target and/or synapse_model is/are given,
    they must be single values, lists of length one or the same length
    as source. Use GetStatus()/SetStatus() to inspect/modify the found
    connections.

    Note: FindConnections() is deprecated and will be removed in the future.
          Use GetConnections() instead.

    Note: synapse_type is alias for synapse_model for backward compatibility
    """

    if synapse_model and synapse_type:
        raise NESTError("synapse_type is alias for synapse_model, cannot be used together.")
    if synapse_type:
        synapse_model = synapse_type

    if not target and not synapse_model:
        params = [{"source": s} for s in source]

    if not target and synapse_model:
        synapse_model = broadcast(synapse_model, len(source), (str,), "synapse_model")
        params = [{"source": s, "synapse_model": syn}
                  for s, syn in zip(source, synapse_model)]

    if target and not synapse_model:
        target = broadcast(target, len(source), (int,), "target")
        params = [{"source": s, "target": t} for s, t in zip(source, target)]

    if target and synapse_model:
        target = broadcast(target, len(source), (int,), "target")
        synapse_model = broadcast(synapse_model, len(source), (str,), "synapse_model")
        params = [{"source": s, "target": t, "synapse_model": syn}
                  for s, t, syn in zip(source, target, synapse_model)]

    sps(params)
    sr("{FindConnections} Map Flatten")
    
    result=spp()
    return [ { 'source':int(r[0]), 'target_thread': int(r[2]),
               'synapse_modelid': int(r[3]), 'port': int(r[4])} for r in result ]


def GetConnections(source=None, target=None, synapse_model=None) :
    """
    Return an array of connection identifiers.
    
    Parameters:
    source - list of source GIDs
    target - list of target GIDs
    synapse_model - string with the synapse model
    
    If GetConnections is called without parameters, all connections
    in the network are returned.

    If a list of source neurons is given, only connections from these
    pre-synaptic neurons are returned.

    If a list of target neurons is given, only connections to these
    post-synaptic neurons are returned.

    If a synapse model is given, only connections with this synapse
    type are returned.

    Any combination of source, target and synapse_model parameters
    is permitted.

    Each connection id is a 5-tuple or, if available, a NumPy
    array with the following fived entries:
    source-gid, target-gid, target-thread, synapse-id, port
    
    Note: Only connections with targets on the MPI process executing
          the command are returned.
    """
    
    params={}
    if source:
        if not is_sequencetype(source):
            raise NESTError("source must be a list of gids.")
        params['source'] = source
    if target:
        if not is_sequencetype(target):
            raise NESTError("target must be a list of gids.")
        params['target'] = target

    sps(params)
    if synapse_model:
        # add model to params dict as literal
        sr('dup /synapse_model /{0} put_d'.format(synapse_model))

    sr("GetConnections")
    
    return spp()


def Connect(pre, post, params=None, delay=None, model="static_synapse"):
    """
    Make one-to-one connections of type model between the nodes in
    pre and the nodes in post. pre and post have to be lists of the
    same length. If params is given (as dictionary or list of
    dictionaries), they are used as parameters for the connections. If
    params is given as a single float or as list of floats, it is used
    as weight(s), in which case delay also has to be given as float or
    as list of floats.
    """

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
    """
    Connect all neurons in pre to each neuron in post. pre and post
    have to be lists. If weight is given (as a single float or as list
    of floats), delay also has to be given as float or as list of
    floats.
    """

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


def RandomConvergentConnect(pre, post, n, weight=None, delay=None, model="static_synapse", options=None):
    """
    Connect n randomly selected neurons from pre to each neuron in
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
    """
    Connect each neuron in pre to all neurons in post. pre and post
    have to be lists. If weight is given (as a single float or as list
    of floats), delay also has to be given as float or as list of
    floats.
    """

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
        cmd='/%s DivergentConnect' % model
        for s in pre :
            sps(s)
            sps(post)
            sps(weight)
            sps(delay)
            sr(cmd)
    
    else:
        raise NESTError("Both 'weight' and 'delay' have to be given.")


def DataConnect(pre, params=None, model=None):
    """
    Connect neurons from lists of connection data.

    Variant 1.
    pre: [gid_1, ... gid_n]
    params: [ {param1}, ..., {param_n} ]
    model= 'synapse_model'

    Variant 2:
    pre = [ {synapse_state1}, ..., {synapse_state_n}]
    params=None
    model=None

    Variant 1 of DataConnect connects each neuron in pre to the targets given in params, using synapse type model.
    The dictionary parames must contain at least the following keys:
    'target'
    'weight'
    'delay'
    each resolving to a list or numpy.ndarray of values. Depending on the synapse model, other parameters can be given
    in the same format. All arrays in params must have the same length as 'target'.

    Variant 2 of DataConnect will connect neurons according to a list of synapse status dictionaries,
    as obtained from GetStatus.
    Note: During connection, status dictionary misses will not raise errors, even if
    the kernel property 'dict_miss_is_error' is True.
    """

    if not is_sequencetype(pre):
        raise NESTError("'pre' must be a list of nodes or connection dictionaries.")
    if params and not is_sequencetype(params):
        raise NESTError("'params' must be a list of dictionaries.")

    if params:
	if not model:
		model="static_synapse"
	cmd='(%s) DataConnect_i_D_s ' % model
    
	for s,p in zip(pre,params):
		sps(s)
		sps(p)
		sr(cmd)
    else:
	    # Call the variant where all connections are
	    # given explicitly
            dictmiss=GetKernelStatus('dict_miss_is_error')
            # We disable dictionary checking now because most models
            # cannot re-use their own status dictionary
            SetKernelStatus({'dict_miss_is_error': False})
	    sps(pre)
	    sr('DataConnect_a')
            SetKernelStatus({'dict_miss_is_error': dictmiss})
            
    
def RandomDivergentConnect(pre, post, n, weight=None, delay=None, model="static_synapse", options=None):
    """
    Connect each neuron in pre to n randomly selected neurons from
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


def CGConnect(pre, post, cg, parameter_map={}, model="static_synapse"):
    """
    Connect neurons from pre to neurons from post using connectivity
    specified by the connection generator cg. pre and post are either
    both lists containing 1 subnet, or lists of gids. parameter_map is
    a dictionary mapping names of values such as weight and delay to
    value set positions.
    """

    if GetStatus(pre[:1], "model")[0] == "subnet":
        if GetStatus(post[:1], "model")[0] != "subnet":
            raise NESTError("if pre is a subnet, post also has to be a subnet")
        if len(pre) > 1 or len(post) > 1:
            raise NESTError("the length of pre and post has to be 1 if subnets are given")
        sli_func('CGConnect', cg, pre[0], post[0], parameter_map, '/'+model, litconv=True)
        
    else:
        sli_func('CGConnect', cg, pre, post, parameter_map, '/'+model, litconv=True)


# -------------------- Functions for hierarchical networks

def PrintNetwork(depth=1, subnet=None) :
    """
    Print the network tree up to depth, starting at subnet. if
    subnet is omitted, the current subnet is used instead.
    """
    
    if subnet == None:
        subnet = CurrentSubnet()
    elif len(subnet) > 1:
        raise NESTError("PrintNetwork() expects exactly one GID.")

    sps(subnet[0])
    sr("%i PrintNetwork" % depth)


def CurrentSubnet() :
    """
    Returns the global id of the current subnet.
    """

    sr("CurrentSubnet")
    return [spp()]


def ChangeSubnet(subnet) :
    """
    Make subnet the current subnet.
    """

    if len(subnet) > 1:
        raise NESTError("ChangeSubnet() expects exactly one GID.")

    sps(subnet[0])
    sr("ChangeSubnet")


def GetLeaves(subnets, properties=None, local_only=False) :
    """
    Return the global ids of the leaf nodes of the given subnets.
    
    Leaf nodes are all nodes that are not subnets.
    
    If properties is given, it must be a dictionary. Only global ids of nodes 
       matching the properties given in the dictionary exactly will be returned.
       Matching properties with float values (e.g. the membrane potential) may
       fail due to tiny numerical discrepancies and should be avoided.
       
    If local_only is True, only global ids of nodes simulated on the local MPI 
       process will be returned. By default, global ids of nodes in the entire
       simulation will be returned. This requires MPI communication and may
       slow down the script.
       
    See also: GetNodes, GetChildren
    """

    if properties is None:
        properties = {}
    func = 'GetLocalLeaves' if local_only else 'GetGlobalLeaves'
    return sli_func('/props Set { props %s } Map' % func, subnets, properties,
                    litconv=True)    


def GetNodes(subnets, properties=None, local_only=False):
    """
    Return the global ids of the all nodes of the given subnets.
    
    If properties is given, it must be a dictionary. Only global ids of nodes 
       matching the properties given in the dictionary exactly will be returned.
       Matching properties with float values (e.g. the membrane potential) may
       fail due to tiny numerical discrepancies and should be avoided.
       
    If local_only is True, only global ids of nodes simulated on the local MPI 
       process will be returned. By default, global ids of nodes in the entire
       simulation will be returned. This requires MPI communication and may
       slow down the script.
       
    See also: GetLeaves, GetChildren
    """

    if properties is None:
        properties = {}
    func = 'GetLocalNodes' if local_only else 'GetGlobalNodes'
    return sli_func('/props Set { props %s } Map' % func, subnets, properties,
                    litconv=True)    


def GetChildren(subnets, properties=None, local_only=False):
    """
    Return the global ids of the immediate children of the given subnets.
    
    If properties is given, it must be a dictionary. Only global ids of nodes 
       matching the properties given in the dictionary exactly will be returned.
       Matching properties with float values (e.g. the membrane potential) may
       fail due to tiny numerical discrepancies and should be avoided.
       
    If local_only is True, only global ids of nodes simulated on the local MPI 
       process will be returned. By default, global ids of nodes in the entire
       simulation will be returned. This requires MPI communication and may
       slow down the script.
       
    See also: GetNodes, GetLeaves
    """

    if properties is None:
        properties = {}
    func = 'GetLocalChildren' if local_only else 'GetGlobalChildren'
    return sli_func('/props Set { props %s } Map' % func, subnets, properties,
                    litconv=True)    

        
def GetNetwork(gid, depth):
    """
    Return a nested list with the children of subnet id at level
    depth. If depth==0, the immediate children of the subnet are
    returned. The returned list is depth+1 dimensional.
    """
    
    if len(gid)>1 :
        raise NESTError("GetNetwork() expects exactly one GID.")
    
    sps(gid[0])
    sps(depth)
    sr("GetNetwork")
    return spp()


def BeginSubnet(label=None, params=None):
    """
    Create a new subnet and change into it. A string argument can be
    used to name the new subnet A dictionary argument can be used to
    set the subnet's custom dict.
    """

    sn=Create("subnet")
    if label:
        SetStatus(sn, "label", label)
    if params:
        SetStatus(sn, "customdict", params)
    ChangeSubnet(sn)


def EndSubnet():
    """
    Change to the parent subnet and return the gid of the current.
    """

    csn=CurrentSubnet()
    parent=GetStatus(csn, "parent")

    if csn != parent:
        ChangeSubnet(parent)
        return csn
    else:
        raise NESTError("Unexpected EndSubnet(). Cannot go higher than the root node.")


def LayoutNetwork(model, dim, label=None, params=None) :
    """
    Create a subnetwork of dimension dim with nodes of type model and
    return a list of ids.
    """

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
