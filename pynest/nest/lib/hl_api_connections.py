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
from .hl_api_subnets import GetChildren
import numpy


@check_stack
def GetConnections(source=None, target=None, synapse_model=None,
                   synapse_label=None):
    """Return an array of connection identifiers.

    Any combination of source, target, synapse_model and
    synapse_label parameters is permitted.

    Parameters
    ----------
    source : list, optional
        Source GIDs, only connections from these
        pre-synaptic neurons are returned
    target : list, optional
        Target GIDs, only connections to these
        post-synaptic neurons are returned
    synapse_model : str, optional
        Only connections with this synapse type are returned
    synapse_label : int, optional
        (non-negative) only connections with this synapse label are returned

    Returns
    -------
    array:
        Connections as 5-tuples with entries
        (source-gid, target-gid, target-thread, synapse-id, port)

    Notes
    -----
    Only connections with targets on the MPI process executing
    the command are returned.


    Raises
    ------
    TypeError
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
def Connect(pre, post, conn_spec=None, syn_spec=None, model=None):
    """
    Connect pre nodes to post nodes.

    Nodes in pre and post are connected using the specified connectivity
    (all-to-all by default) and synapse type (static_synapse by default).
    Details depend on the connectivity rule.

    Parameters
    ----------
    pre : list
        Presynaptic nodes, as list of GIDs
    post : list
        Postsynaptic nodes, as list of GIDs
    conn_spec : str or dict, optional
        Specifies connectivity rule, see below
    syn_spec : str or dict, optional
        Specifies synapse model, see below
    model : str or dict, optional
        alias for syn_spec for backward compatibility

    Raises
    ------
    kernel.NESTError

    Notes
    -----
    Connect does not iterate over subnets, it only connects explicitly
    specified nodes.

    Connectivity specification (conn_spec)
    --------------------------------------

    Connectivity is specified either as a string containing the name of a
    connectivity rule (default: 'all_to_all') or as a dictionary specifying
    the rule and any mandatory rule-specific parameters (e.g. 'indegree').

    In addition, switches setting permission for establishing
    self-connections ('autapses', default: True) and multiple connections
    between a pair of nodes ('multapses', default: True) can be contained
    in the dictionary. Another switch enables the creation of symmetric
    connections ('symmetric', default: False) by also creating connections
    in the opposite direction.

    Available rules and associated parameters
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    - 'all_to_all' (default)
    - 'one_to_one'
    - 'fixed_indegree', 'indegree'
    - 'fixed_outdegree', 'outdegree'
    - 'fixed_total_number', 'N'
    - 'pairwise_bernoulli', 'p'

    Example conn-spec choices
    ~~~~~~~~~~~~~~~~~~~~~~~~~
    - 'one_to_one'
    - {'rule': 'fixed_indegree', 'indegree': 2500, 'autapses': False}
    - {'rule': 'pairwise_bernoulli', 'p': 0.1}

    Synapse specification (syn_spec)
    --------------------------------------

    The synapse model and its properties can be given either as a string
    identifying a specific synapse model (default: 'static_synapse') or
    as a dictionary specifying the synapse model and its parameters.

    Available keys in the synapse specification dictionary are:
    - 'model'
    - 'weight'
    - 'delay'
    - 'receptor_type'
    - any parameters specific to the selected synapse model.

    All parameters are optional and if not specified, the default values
    of the synapse model will be used. The key 'model' identifies the
    synapse model, this can be one of NEST's built-in synapse models
    or a user-defined model created via CopyModel().

    If 'model' is not specified the default model 'static_synapse'
    will be used.

    All other parameters can be scalars, arrays or distributions.
    In the case of scalar parameters, all keys must be doubles
    except for 'receptor_type' which must be initialised with an integer.

    Parameter arrays are available for the rules 'one_to_one',
    'all_to_all', 'fixed_indegree' and 'fixed_outdegree':
    - For 'one_to_one' the array has to be a one-dimensional
      NumPy array with length len(pre).
    - For 'all_to_all' the array has to be a two-dimensional NumPy array
      with shape (len(post), len(pre)), therefore the rows describe the
      target and the columns the source neurons.
    - For 'fixed_indegree' the array has to be a two-dimensional NumPy array
      with shape (len(post), indegree), where indegree is the number of
      incoming connections per target neuron, therefore the rows describe the
      target and the columns the connections converging to the target neuron,
      regardless of the identity of the source neurons.
    - For 'fixed_outdegree' the array has to be a two-dimensional NumPy array
      with shape (len(pre), outdegree), where outdegree is the number of
      outgoing connections per source neuron, therefore the rows describe the
      source and the columns the connections starting from the source neuron
      regardless of the identity of the target neuron.

    Any distributed parameter must be initialised with a further dictionary
    specifying the distribution type ('distribution', e.g. 'normal') and
    any distribution-specific parameters (e.g. 'mu' and 'sigma').

    To see all available distributions, run:
    nest.slirun('rdevdict info')

    To get information on a particular distribution, e.g. 'binomial', run:
    nest.help('rdevdict::binomial')

    Most common available distributions and associated parameters
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    - 'normal' with 'mu', 'sigma'
    - 'normal_clipped' with 'mu', 'sigma', 'low', 'high'
    - 'lognormal' with 'mu', 'sigma'
    - 'lognormal_clipped' with 'mu', 'sigma', 'low', 'high'
    - 'uniform' with 'low', 'high'
    - 'uniform_int' with 'low', 'high'

    Example syn-spec choices
    ~~~~~~~~~~~~~~~~~~~~~~~~~
    - 'stdp_synapse'
    - {'weight': 2.4, 'receptor_type': 1}
    - {'model': 'stdp_synapse',
       'weight': 2.5,
       'delay': {'distribution': 'uniform', 'low': 0.8, 'high': 2.5},
       'alpha': {
           'distribution': 'normal_clipped', 'low': 0.5,
           'mu': 5.0, 'sigma': 1.0}
      }
    """

    if model is not None:
        deprecation_text = "".join([
            "The argument 'model' is there for backward compatibility with ",
            "the old Connect function and will be removed in a future",
            "version of NEST. Please change the name of the keyword argument ",
            "from 'model' to 'syn_spec'. For details, see the documentation ",
            "at:\nhttp://www.nest-simulator.org/connection_management"
        ])
        show_deprecation_warning("BackwardCompatibilityConnect",
                                 text=deprecation_text)

    if model is not None and syn_spec is not None:
        raise kernel.NESTError(
            "'model' is an alias for 'syn_spec' and cannot "
            "be used together with 'syn_spec'.")

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
            raise kernel.NESTError(
                "conn_spec needs to be a string or dictionary.")
    else:
        sr('/Connect /conn_spec GetOption')

    if model is not None:
        syn_spec = model

    if syn_spec is not None:
        if is_string(syn_spec):
            sps(syn_spec)
            sr("cvlit")
        elif isinstance(syn_spec, dict):
            for key, value in syn_spec.items():

                # if value is a list, it is converted to a numpy array
                if isinstance(value, (list, tuple)):
                    value = numpy.asarray(value)

                if isinstance(value, (numpy.ndarray, numpy.generic)):

                    if len(value.shape) == 1:
                        if rule == 'one_to_one':
                            if value.shape[0] != len(pre):
                                raise kernel.NESTError(
                                    "'" + key + "' has to be an array of "
                                    "dimension " + str(len(pre)) + ", a "
                                    "scalar or a dictionary.")
                            else:
                                syn_spec[key] = value
                        else:
                            raise kernel.NESTError(
                                "'" + key + "' has the wrong type. "
                                "One-dimensional parameter arrays can "
                                "only be used in conjunction with rule "
                                "'one_to_one'.")

                    elif len(value.shape) == 2:
                        if rule == 'all_to_all':
                            if value.shape[0] != len(post) or \
                                    value.shape[1] != len(pre):

                                raise kernel.NESTError(
                                    "'" + key + "' has to be an array of "
                                    "dimension " + str(len(post)) + "x" +
                                    str(len(pre)) +
                                    " (n_target x n_sources), " +
                                    "a scalar or a dictionary.")
                            else:
                                syn_spec[key] = value.flatten()
                        elif rule == 'fixed_indegree':
                            indegree = conn_spec['indegree']
                            if value.shape[0] != len(post) or \
                                    value.shape[1] != indegree:
                                raise kernel.NESTError(
                                    "'" + key + "' has to be an array of "
                                    "dimension " + str(len(post)) + "x" +
                                    str(indegree) +
                                    " (n_target x indegree), " +
                                    "a scalar or a dictionary.")
                            else:
                                syn_spec[key] = value.flatten()
                        elif rule == 'fixed_outdegree':
                            outdegree = conn_spec['outdegree']
                            if value.shape[0] != len(pre) or \
                                    value.shape[1] != outdegree:
                                raise kernel.NESTError(
                                    "'" + key + "' has to be an array of "
                                    "dimension " + str(len(pre)) + "x" +
                                    str(outdegree) +
                                    " (n_sources x outdegree), " +
                                    "a scalar or a dictionary.")
                            else:
                                syn_spec[key] = value.flatten()
                        else:
                            raise kernel.NESTError(
                                "'" + key + "' has the wrong type. "
                                "Two-dimensional parameter arrays can "
                                "only be used in conjunction with rules "
                                "'all_to_all', 'fixed_indegree' or "
                                "'fixed_outdegree'.")
            sps(syn_spec)
        else:
            raise kernel.NESTError(
                "syn_spec needs to be a string or dictionary.")

    sr('Connect')


@check_stack
@deprecated('', 'DataConnect is deprecated and will be removed in NEST 3.0.\
Use Connect() with one_to_one rule instead.')
def DataConnect(pre, params=None, model="static_synapse"):
    """Connect neurons from lists of connection data.

    Parameters
    ----------
    pre : list
        Presynaptic nodes, given as lists of GIDs or lists
        of synapse status dictionaries. See below.
    params : list, optional
        See below
    model : str, optional
        Synapse model to use, see below

    Raises
    ------
    TypeError

    Usage Variants
    --------------

    Variant 1
    ~~~~~~~~~

    Connect each neuron in pre to the targets given in params,
    using synapse type model.

    - pre: [gid_1, ... gid_n]
    - params: [ {param_1}, ..., {param_n} ]
    - model= 'synapse_model'

    The dictionaries param_1 to param_n must contain at least the
    following keys:
    - 'target'
    - 'weight'
    - 'delay'
    Each key must resolve to a list or numpy.ndarray of values.

    Depending on the synapse model, other parameters can be given
    in the same format. All arrays in params must have the same
    length as 'target'.

    Variant 2
    ~~~~~~~~~

    Connect neurons according to a list of synapse status dictionaries,
    as obtained from GetStatus.

    pre = [ {synapse_state1}, ..., {synapse_state_n}]
    params=None
    model=None

    During connection, status dictionary misses will not raise errors,
    even if the kernel property 'dict_miss_is_error' is True.
    """

    if not is_coercible_to_sli_array(pre):
        raise TypeError(
            "pre must be a list of nodes or connection dictionaries")

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
        # Disable dict checking, because most models can't re-use
        # their own status dict

        dict_miss = GetKernelStatus('dict_miss_is_error')
        SetKernelStatus({'dict_miss_is_error': False})

        sps(pre)
        sr('DataConnect_a')

        SetKernelStatus({'dict_miss_is_error': dict_miss})


@check_stack
def CGConnect(pre, post, cg, parameter_map=None, model="static_synapse"):
    """Connect neurons using the Connection Generator Interface.

    Potential pre-synaptic neurons are taken from pre, potential
    post-synaptic neurons are taken from post. The connection
    generator cg specifies the exact connectivity to be set up. The
    parameter_map can either be None or a dictionary that maps the
    keys "weight" and "delay" to their integer indices in the value
    set of the connection generator.

    This function is only available if NEST was compiled with
    support for libneurosim.

    For further information, see
    * The NEST documentation on using the CG Interface at
      http://nest-simulator.org/connection-generator-interface
    * The GitHub repository and documentation for libneurosim at
      https://github.com/INCF/libneurosim/
    * The publication about the Connection Generator Interface at
      https://doi.org/10.3389/fninf.2014.00043

    Parameters
    ----------
    pre : list or numpy.array
        must contain a list of GIDs
    post : list or numpy.array
        must contain a list of GIDs
    cg : connection generator
        libneurosim connection generator to use
    parameter_map : dict, optional
        Maps names of values such as weight and delay to
        value set positions
    model : str, optional
        Synapse model to use

    Raises
    ------
    kernel.NESTError
    """

    sr("statusdict/have_libneurosim ::")
    if not spp():
        raise kernel.NESTError(
            "NEST was not compiled with support for libneurosim: " +
            "CGConnect is not available.")

    if parameter_map is None:
        parameter_map = {}

    sli_func('CGConnect', cg, pre, post,
             parameter_map, '/' + model, litconv=True)


@check_stack
def CGParse(xml_filename):
    """Parse an XML file and return the corresponding connection
    generator cg.

    The library to provide the parsing can be selected
    by CGSelectImplementation().

    Parameters
    ----------
    xml_filename : str
        Filename of the xml file to parse.

    Raises
    ------
    kernel.NESTError
    """

    sr("statusdict/have_libneurosim ::")
    if not spp():
        raise kernel.NESTError(
            "NEST was not compiled with support for libneurosim: " +
            "CGParse is not available.")

    sps(xml_filename)
    sr("CGParse")
    return spp()


@check_stack
def CGSelectImplementation(tag, library):
    """Select a library to provide a parser for XML files and associate
    an XML tag with the library.

    XML files can be read by CGParse().

    Parameters
    ----------
    tag : str
        XML tag to associate with the library
    library : str
        Library to use to parse XML files

    Raises
    ------
    kernel.NESTError
    """

    sr("statusdict/have_libneurosim ::")
    if not spp():
        raise kernel.NESTError(
            "NEST was not compiled with support for libneurosim: " +
            "CGSelectImplementation is not available.")

    sps(tag)
    sps(library)
    sr("CGSelectImplementation")


@check_stack
@deprecated('', 'DisconnectOneToOne is deprecated and will be removed in \
NEST-3.0. Use Disconnect instead.')
def DisconnectOneToOne(source, target, syn_spec):
    """Disconnect a currently existing synapse.

    Parameters
    ----------
    source : int
        GID of presynaptic node
    target : int
        GID of postsynaptic node
    syn_spec : str or dict
        See Connect() for definition
    """

    sps(source)
    sps(target)
    if is_string(syn_spec):
        syn_spec = {'model': syn_spec}
    sps(syn_spec)
    sr('Disconnect')


@check_stack
def Disconnect(pre, post, conn_spec='one_to_one', syn_spec='static_synapse'):
    """Disconnect pre neurons from post neurons.

    Neurons in pre and post are disconnected using the specified disconnection
    rule (one-to-one by default) and synapse type (static_synapse by default).
    Details depend on the disconnection rule.

    Parameters
    ----------
    pre : list
        Presynaptic nodes, given as list of GIDs
    post : list
        Postsynaptic nodes, given as list of GIDs
    conn_spec : str or dict
        Disconnection rule, see below
    syn_spec : str or dict
        Synapse specifications, see below

    conn_spec
    ---------
    Apply the same rules as for connectivity specs in the Connect method

    Possible choices of the conn_spec are
    - 'one_to_one'
    - 'all_to_all'

    syn_spec
    --------
    The synapse model and its properties can be inserted either as a
    string describing one synapse model (synapse models are listed in the
    synapsedict) or as a dictionary as described below.

    Note that only the synapse type is checked when we disconnect and that if
    syn_spec is given as a non-empty dictionary, the 'model' parameter must be
    present.

    If no synapse model is specified the default model 'static_synapse'
    will be used.

    Available keys in the synapse dictionary are:
    - 'model'
    - 'weight'
    - 'delay',
    - 'receptor_type'
    - parameters specific to the synapse model chosen

    All parameters are optional and if not specified will use the default
    values determined by the current synapse model.

    'model' determines the synapse type, taken from pre-defined synapse
    types in NEST or manually specified synapses created via CopyModel().

    All other parameters are not currently implemented.

    Notes
    -----
    Disconnect does not iterate over subnets, it only disconnects explicitly
    specified nodes.
    """

    sps(pre)
    sr('cvgidcollection')
    sps(post)
    sr('cvgidcollection')

    if is_string(conn_spec):
        conn_spec = {'rule': conn_spec}
    if is_string(syn_spec):
        syn_spec = {'model': syn_spec}

    sps(conn_spec)
    sps(syn_spec)

    sr('Disconnect_g_g_D_D')
