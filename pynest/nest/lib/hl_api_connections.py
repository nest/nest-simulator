# -*- coding: utf-8 -*-
#
# hl_api_connections.py
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
Functions for connection handling
"""

from .hl_api_helper import *
from .hl_api_nodes import Create
from .hl_api_info import GetStatus
from .hl_api_simulation import GetKernelStatus, SetKernelStatus

import numpy

@check_stack
@deprecated(alt_func_name='GetConnections')
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

    if synapse_model is not None and synapse_type is not None:
        raise kernel.NESTError("'synapse_type' is alias for 'synapse_model' and cannot be used together with 'synapse_model'.")

    if synapse_type is not None:
        synapse_model = synapse_type

    if target is None and synapse_model is None:
        params = [{"source": s} for s in source]
    elif target is None and synapse_model is not None:
        synapse_model = broadcast(synapse_model, len(source), (uni_str,), "synapse_model")
        params = [{"source": s, "synapse_model": syn}
                  for s, syn in zip(source, synapse_model)]
    elif target is not None and synapse_model is None:
        target = broadcast(target, len(source), (int,), "target")
        params = [{"source": s, "target": t} for s, t in zip(source, target)]
    else:  # target is not None and synapse_model is not None
        target = broadcast(target, len(source), (int,), "target")
        synapse_model = broadcast(synapse_model, len(source), (uni_str,), "synapse_model")
        params = [{"source": s, "target": t, "synapse_model": syn}
                  for s, t, syn in zip(source, target, synapse_model)]

    sps(params)
    sr("{FindConnections} Map Flatten")

    result = ({
        'source': int(src),
        'target_thread': int(tt),
        'synapse_modelid': int(sm),
        'port': int(prt)
    } for src, _, tt, sm, prt in spp())

    return tuple(result)


@check_stack
def GetConnections(source=None, target=None, synapse_model=None, synapse_label=None):
    """
    Return an array of connection identifiers.
    
    Parameters:
    source - list of source GIDs
    target - list of target GIDs
    synapse_model - string with the synapse model
    synapse_label - non negative integer with synapse label
    
    If GetConnections is called without parameters, all connections
    in the network are returned.

    If a list of source neurons is given, only connections from these
    pre-synaptic neurons are returned.

    If a list of target neurons is given, only connections to these
    post-synaptic neurons are returned.

    If a synapse model is given, only connections with this synapse
    type are returned.
    
    If a synapse label is given, only connections with this synapse
    label are returned.

    Any combination of source, target, synapse_model and synapse_label
    parameters is permitted.

    Each connection id is a 5-tuple or, if available, a NumPy
    array with the following five entries:
    source-gid, target-gid, target-thread, synapse-id, port
    
    Note: Only connections with targets on the MPI process executing
          the command are returned.
    """

    params = {}

    if source is not None:
        if not is_coercible_to_sli_array(source):
            raise TypeError("source must be a list of GIDs")
        params['source'] = source

    if target is not None:
        if not is_coercible_to_sli_array(target):
            raise TypeError("target must be a list of GIDs")
        params['target'] = target

    if synapse_model is not None:
        params['synapse_model'] = kernel.SLILiteral(synapse_model)

    if synapse_label is not None:
        params['synapse_label'] = synapse_label

    sps(params)
    sr("GetConnections")

    return spp()


@check_stack
@deprecated(alt_func_name='Connect')
def OneToOneConnect(pre, post, params=None, delay=None, model="static_synapse"):
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
        raise kernel.NESTError("pre and post have to be the same length")

    # pre post Connect
    if params is None and delay is None:
        for s,d in zip(pre, post):
            sps(s)
            sps(d)
            sr('/%s Connect' % model)

    # pre post params Connect
    elif params is not None and delay is None:
        params = broadcast(params, len(pre), (dict,), "params")
        if len(params) != len(pre):
            raise kernel.NESTError("params must be a dict, or list of dicts of length 1 or len(pre).")

        for s,d,p in zip(pre, post, params) :
            sps(s)
            sps(d)
            sps(p)
            sr('/%s Connect' % model)

    # pre post w d Connect
    elif params is not None and delay is not None:
        params = broadcast(params, len(pre), (float,), "params")
        if len(params) != len(pre):
            raise kernel.NESTError("params must be a float, or list of floats of length 1 or len(pre) and will be used as weight(s).")
        delay = broadcast(delay, len(pre), (float,), "delay")
        if len(delay) != len(pre):
            raise kernel.NESTError("delay must be a float, or list of floats of length 1 or len(pre).")

        for s,d,w,dl in zip(pre, post, params, delay) :
            sps(s)
            sps(d)
            sps(w)
            sps(dl)
            sr('/%s Connect' % model)

    else:
        raise kernel.NESTError("Both 'params' and 'delay' have to be given.")


@check_stack
@deprecated(alt_func_name='Connect')
def ConvergentConnect(pre, post, weight=None, delay=None, model="static_synapse"):
    """
    Connect all neurons in pre to each neuron in post. pre and post
    have to be lists. If weight is given (as a single float or as list
    of floats), delay also has to be given as float or as list of
    floats.
    """

    if weight is None and delay is None:
        for d in post :
            sps(pre)
            sps(d)
            sr('/%s ConvergentConnect' % model)

    elif weight is not None and delay is not None:
        weight = broadcast(weight, len(pre), (float,), "weight")
        if len(weight) != len(pre):
            raise kernel.NESTError("weight must be a float, or sequence of floats of length 1 or len(pre)")
        delay = broadcast(delay, len(pre), (float,), "delay")
        if len(delay) != len(pre):
            raise kernel.NESTError("delay must be a float, or sequence of floats of length 1 or len(pre)")
        
        for d in post:
            sps(pre)
            sps(d)
            sps(weight)
            sps(delay)
            sr('/%s ConvergentConnect' % model)

    else:
        raise kernel.NESTError("Both 'weight' and 'delay' have to be given.")


@check_stack
@deprecated(alt_func_name='Connect')
def RandomConvergentConnect(pre, post, n, weight=None, delay=None, model="static_synapse", options=None):
    """
    Connect n randomly selected neurons from pre to each neuron in
    post. pre and post have to be lists. If weight is given (as a
    single float or as list of floats), delay also has to be given as
    float or as list of floats. options is a dictionary specifying
    options to the RandomConvergentConnect function: allow_autapses,
    allow_multapses.
    """

    if not isinstance(n, int):
        raise TypeError("number of neurons n should be an integer")

    # store current options, set desired options
    old_options = None
    error = False
    if options is not None:
        old_options = sli_func('GetOptions', '/RandomConvergentConnect',
                               litconv=True)
        del old_options['DefaultOptions'] # in the way when restoring
        sli_func('SetOptions', '/RandomConvergentConnect', options,
                 litconv=True)

    if weight is None and delay is None:
        sli_func(
            '/m Set /n Set /pre Set { pre exch n m RandomConvergentConnect } forall',
            post, pre, n, '/'+model, litconv=True)
    
    elif weight is not None and delay is not None:
        weight = broadcast(weight, n, (float,), "weight")
        if len(weight) != n:
            raise kernel.NESTError("weight must be a float, or sequence of floats of length 1 or n")
        delay = broadcast(delay, n, (float,), "delay")
        if len(delay) != n:
            raise kernel.NESTError("delay must be a float, or sequence of floats of length 1 or n")

        sli_func(
            '/m Set /d Set /w Set /n Set /pre Set { pre exch n w d m RandomConvergentConnect } forall',
            post, pre, n, weight, delay, '/'+model, litconv=True)
    
    else:
        error = True

    # restore old options
    if old_options is not None:
        sli_func('SetOptions', '/RandomConvergentConnect', old_options,
                 litconv=True)

    if error:
        raise kernel.NESTError("Both 'weight' and 'delay' have to be given.")


@check_stack
@deprecated(alt_func_name='Connect')
def DivergentConnect(pre, post, weight=None, delay=None, model="static_synapse"):
    """
    Connect each neuron in pre to all neurons in post. pre and post
    have to be lists. If weight is given (as a single float or as list
    of floats), delay also has to be given as float or as list of
    floats.
    """

    if weight is None and delay is None:
        for s in pre :
            sps(s)
            sps(post)
            sr('/%s DivergentConnect' % model)

    elif weight is not None and delay is not None:
        weight = broadcast(weight, len(post), (float,), "weight")
        if len(weight) != len(post):
            raise kernel.NESTError("weight must be a float, or sequence of floats of length 1 or len(post)")
        delay = broadcast(delay, len(post), (float,), "delay")
        if len(delay) != len(post):
            raise kernel.NESTError("delay must be a float, or sequence of floats of length 1 or len(post)")
        cmd='/%s DivergentConnect' % model
        for s in pre :
            sps(s)
            sps(post)
            sps(weight)
            sps(delay)
            sr(cmd)
    
    else:
        raise kernel.NESTError("Both 'weight' and 'delay' have to be given.")


@check_stack
def Connect(pre, post, conn_spec=None, syn_spec=None, model=None):
    """
    Connect pre nodes to post nodes.

    Nodes in pre and post are connected using the specified connectivity
    (all-to-all by default) and synapse type (static_synapse by default).
    Details depend on the connectivity rule.

    Note:
    Connect does not iterate over subnets, it only connects explicitly
    specified nodes.

    pre - presynaptic nodes, given as list of GIDs
    post - presynaptic nodes, given as list of GIDs
    conn_spec - string, or dictionary specifying connectivity rule, see below
    syn_spec - string, or dictionary specifying synapse model, see below

    Connectivity specification (conn_spec):
    
    Connectivity is specified either as a string containing the name of a
    connectivity rule (default: 'all_to_all') or as a dictionary specifying
    the rule and any mandatory rule-specific parameters (e.g. 'indegree').
    In addition, switches setting permission for establishing self-connections 
    ('autapses', default: True) and multiple connections between a pair of nodes 
    ('multapses', default: True) can be contained in the dictionary.

    Available rules and their associated parameters are:
     - 'all_to_all' (default)
     - 'one_to_one'
     - 'fixed_indegree', 'indegree'
     - 'fixed_outdegree', 'outdegree'
     - 'fixed_total_number', 'N'
     - 'pairwise_bernoulli', 'p'

    Example choices for the conn_spec are:
    - 'one_to_one'
    - {'rule': 'fixed_indegree', 'indegree': 2500, 'autapses': False}
    - {'rule': 'pairwise_bernoulli', 'p': 0.1}

    Synapse specification (syn_spec):

    The synapse model and its properties can be given either as a string identifying
    a specific synapse model (default: 'static_synapse') or as a dictionary 
    specifying the synapse model and its parameters. 
    
    Available keys in the synapse specification dictionary are 'model', 'weight',
    'delay', 'receptor_type' and any parameters specific to the selected synapse model.
    All parameters are optional and if not specified, the default values of the synapse 
    model will be used. The key 'model' identifies the synapse model, this can be one
    of NEST's built-in synapse models or a user-defined model created via CopyModel().
    If 'model' is not specified the default model 'static_synapse' will be used.

    All other parameters can be scalars, arrays or distributions. 
    In the case of scalar parameters, all keys must be doubles except for 'receptor_type' which must be
    initialised with an integer. 
    Parameter arrays are only available for the rules 'one_to_one' and 'all_to_all'. For 'one_to_one' the
    array has to be a one-dimensional NumPy array with length len(pre). For 'all_to_all' the array has
    to be a two-dimensional NumPy array with shape (len(post), len(pre)), therefore the rows describe the 
    target and the columns the source neurons.
    Any distributed parameter must be initialised with a further dictionary specifying the distribution 
    type ('distribution', e.g. 'normal') and any distribution-specific parameters (e.g. 'mu' and 'sigma').

    Available distributions are given in the rdevdict, the most common ones (and their
    associated parameters) are:

    - 'normal' with 'mu', 'sigma'
    - 'normal_clipped' with 'mu', 'sigma', 'low', 'high'
    - 'lognormal' with 'mu', 'sigma'
    - 'lognormal_clipped' with 'mu', 'sigma', 'low', 'high'
    - 'uniform' with 'low', 'high'
    - 'uniform_int' with 'low', 'high'

    To see all available distributions, run: 
    nest.slirun(’rdevdict info’)
    To get information on a particular distribution, e.g. 'binomial', run: 
    nest.help(’rdevdict::binomial’)
    
    Example choices for the syn_spec are:
    - 'stdp_synapse'
    - {'weight': 2.4, 'receptor_type': 1}
    - {'model': 'stdp_synapse',
       'weight': 2.5,
       'delay': {'distribution': 'uniform', 'low': 0.8, 'high': 2.5},
       'alpha': {'distribution': 'normal_clipped', 'low': 0.5, 'mu': 5.0, 'sigma': 1.0}
      }

    Note: model is alias for syn_spec for backward compatibility.  
    """

    if model is not None:
        deprecation_text = "".join(["The argument 'model' is there for backward compatibility with the old ",
                                    "Connect function and will be removed in a future version of NEST. Please change the name ",
                                    "of the keyword argument from 'model' to 'syn_spec'. For details, see the ",
                                    "documentation at:\nhttp://www.nest-simulator.org/connection_management"])
        show_deprecation_warning("BackwardCompatibilityConnect", 
                                 text=deprecation_text)

    if model is not None and syn_spec is not None:
        raise kernel.NESTError("'model' is an alias for 'syn_spec' and cannot be used together with 'syn_spec'.")

    sps(pre)
    sps(post)

    # default rule 
    rule = 'all_to_all'

    if conn_spec is not None:
        sps(conn_spec)
        if is_string(conn_spec):
            rule = conn_spec
            sr("cvlit")
        elif isinstance(conn_spec, dict):
            rule = conn_spec['rule']          
        else:
            raise kernel.NESTError("conn_spec needs to be a string or dictionary.")
    else:
        sr('/Connect /conn_spec GetOption')

    if model is not None:
        syn_spec = model

    if syn_spec is not None:
        if is_string(syn_spec):
            sps(syn_spec)
            sr("cvlit")
        elif isinstance(syn_spec, dict):
            for key,value in syn_spec.items():

                # if value is a list, it is converted to a numpy array
                if isinstance(value, (list, tuple)):
                    value = numpy.asarray(value)

                if isinstance(value, (numpy.ndarray, numpy.generic)):
                    if len(value.shape) == 1:
                        if rule == 'one_to_one':
                            if value.shape[0] != len(pre):
                                raise kernel.NESTError("'" + key + "' has to be an array of dimension " + str(len(pre)) + ", a scalar or a dictionary.")
                            else:
                                syn_spec[key] = value
                        else:
                            raise kernel.NESTError("'" + key + "' has the wrong type. One-dimensional parameter arrays can only be used in conjunction with rule 'one_to_one'.")
                    elif len(value.shape) == 2:
                        if rule == 'all_to_all':
                            if value.shape[0] != len(post) or value.shape[1] != len(pre):
                                raise kernel.NESTError("'" + key + "' has to be an array of dimension " + str(len(post)) + "x" + str(len(pre)) + " (n_target x n_sources), a scalar or a dictionary.")
                            else:
                                syn_spec[key] = value.flatten()
                        else:
                            raise kernel.NESTError("'" + key + "' has the wrong type. Two-dimensional parameter arrays can only be used in conjunction with rule 'all_to_all'.")
            sps(syn_spec)
        else:
            raise kernel.NESTError("syn_spec needs to be a string or dictionary.")

    sr('Connect')


@check_stack
def DataConnect(pre, params=None, model="static_synapse"):
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

    if not is_coercible_to_sli_array(pre):
        raise TypeError("pre must be a list of nodes or connection dictionaries")

    if params is not None:

        if not is_coercible_to_sli_array(params):
            raise TypeError("params must be a list of dictionaries")

        cmd = '({0}) DataConnect_i_D_s '.format(model)

        for s, p in zip(pre, params):
            sps(s)
            sps(p)
            sr(cmd)
    else:
        # Call the variant where all connections are given explicitly

        # Disable dict checking, because most models can't re-use their own status dict
        dict_miss = GetKernelStatus('dict_miss_is_error')
        SetKernelStatus({'dict_miss_is_error': False})

        sps(pre)
        sr('DataConnect_a')

        SetKernelStatus({'dict_miss_is_error': dict_miss})


@check_stack
@deprecated(alt_func_name='Connect')
def RandomDivergentConnect(pre, post, n, weight=None, delay=None, model="static_synapse", options=None):
    """
    Connect each neuron in pre to n randomly selected neurons from
    post. pre and post have to be lists. If weight is given (as a
    single float or as list of floats), delay also has to be given as
    float or as list of floats. options is a dictionary specifying
    options to the RandomDivergentConnect function: allow_autapses,
    allow_multapses.
    """

    if not isinstance(n, int):
        raise TypeError("number of neurons n should be an integer")

    # store current options, set desired options
    old_options = None
    error = False
    if options is not None:
        old_options = sli_func('GetOptions', '/RandomDivergentConnect',
                               litconv=True)
        del old_options['DefaultOptions'] # in the way when restoring
        sli_func('SetOptions', '/RandomDivergentConnect', options,
                 litconv=True)

    if weight is None and delay is None:
        sli_func(
            '/m Set /n Set /post Set { n post m RandomDivergentConnect } forall',
            pre, post, n, '/'+model, litconv=True)

    elif weight is not None and delay is not None:
        weight = broadcast(weight, n, (float,), "weight")
        if len(weight) != n:
            raise kernel.NESTError("weight must be a float, or sequence of floats of length 1 or n")
        delay = broadcast(delay, n, (float,), "delay")
        if len(delay) != n:
            raise kernel.NESTError("delay must be a float, or sequence of floats of length 1 or n")

        sli_func(
            '/m Set /d Set /w Set /n Set /post Set { n post w d m RandomDivergentConnect } forall',
            pre, post, n, weight, delay, '/'+model, litconv=True)

    else:
        error = True

    # restore old options
    if old_options is not None:
        sli_func('SetOptions', '/RandomDivergentConnect', old_options,
                 litconv=True)

    if error:
        raise kernel.NESTError("Both 'weight' and 'delay' have to be given.")

def _is_subnet_instance(gids):
    "Returns true if all gids point to subnet or derived type."

    try:
        GetChildren(gids)
        return True
    except kernel.NESTError:
        return False

@check_stack
def CGConnect(pre, post, cg, parameter_map=None, model="static_synapse"):
    """
    Connect neurons from pre to neurons from post using connectivity
    specified by the connection generator cg. pre and post are either
    both lists containing 1 subnet, or lists of gids. parameter_map is
    a dictionary mapping names of values such as weight and delay to
    value set positions. This function is only available if NEST was
    compiled with support for libneurosim.
    """

    sr("statusdict/have_libneurosim ::")
    if not spp():
        raise kernel.NESTError("NEST was not compiled with support for libneurosim: CGConnect is not available.")

    if parameter_map is None:
        parameter_map = {}

    if _is_subnet_instance(pre[:1]):
        if not _is_subnet_instance(post[:1]):
            raise kernel.NESTError("if pre is a subnet, post also has to be a subnet")
        if len(pre) > 1 or len(post) > 1:
            raise kernel.NESTError("the length of pre and post has to be 1 if subnets are given")
        sli_func('CGConnect', cg, pre[0], post[0], parameter_map, '/'+model, litconv=True)
        
    else:
        sli_func('CGConnect', cg, pre, post, parameter_map, '/'+model, litconv=True)

@check_stack
def CGParse(xml_filename):
    """
    Parse an XML file and return the correcponding connection
    generator cg. The library to provide the parsing can be selected
    by CGSelectImplementation().
    """

    sr("statusdict/have_libneurosim ::")
    if not spp():
        raise kernel.NESTError("NEST was not compiled with support for libneurosim: CGParse is not available.")

    sps(xml_filename)
    sr("CGParse")
    return spp()

@check_stack
def CGSelectImplementation(tag, library):
    """
    Select a library to provide a parser for XML files and associate
    an XML tag with the library. XML files can be read by CGParse().
    """

    sr("statusdict/have_libneurosim ::")
    if not spp():
        raise kernel.NESTError("NEST was not compiled with support for libneurosim: CGSelectImplementation is not available.")

    sps(tag)
    sps(library)
    sr("CGSelectImplementation")
