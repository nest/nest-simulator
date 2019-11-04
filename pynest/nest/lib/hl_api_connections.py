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

import numpy
import warnings

from ..ll_api import *
from .. import pynestkernel as kernel
from .hl_api_helper import *
from .hl_api_nodes import Create
from .hl_api_types import GIDCollection, Connectome, Mask, Parameter
from .hl_api_info import GetStatus
from .hl_api_simulation import GetKernelStatus, SetKernelStatus

__all__ = [
    'CGConnect',
    'CGParse',
    'CGSelectImplementation',
    'Connect',
    'Disconnect',
    'GetConnections',
]


@check_stack
def GetConnections(source=None, target=None, synapse_model=None,
                   synapse_label=None):
    """Return a `Connectome` representing the connection identifiers.

    Any combination of source, target, synapse_model and
    synapse_label parameters is permitted.

    Parameters
    ----------
    source : GIDCollection or list, optional
        Source GIDs, only connections from these
        pre-synaptic neurons are returned
    target : GIDCollection or list, optional
        Target GIDs, only connections to these
        post-synaptic neurons are returned
    synapse_model : str, optional
        Only connections with this synapse type are returned
    synapse_label : int, optional
        (non-negative) only connections with this synapse label are returned

    Returns
    -------
    Connectome:
        Object representing the source-gid, target-gid, target-thread, synapse-id, port of connections, see
        :py:class:`Connectome`.

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
        if isinstance(source, GIDCollection):
            params['source'] = source
        else:
            raise TypeError("source must be GIDCollection.")

    if target is not None:
        if isinstance(target, GIDCollection):
            params['target'] = target
        else:
            raise TypeError("target must be GIDCollection.")

    if synapse_model is not None:
        params['synapse_model'] = kernel.SLILiteral(synapse_model)

    if synapse_label is not None:
        params['synapse_label'] = synapse_label

    sps(params)
    sr("GetConnections")

    conns = spp()

    if isinstance(conns, tuple):
        conns = Connectome(None)

    return conns


def _process_conn_spec(conn_spec):
    if conn_spec is None:
        # Get default conn_spec
        sr('/Connect /conn_spec GetOption')
        return spp()
    elif isinstance(conn_spec, str):
        processed_conn_spec = {'rule': conn_spec}
        return processed_conn_spec
    elif isinstance(conn_spec, dict):
        return conn_spec
    else:
        raise TypeError("conn_spec must be a string or dict")


def _process_syn_spec(syn_spec, conn_spec, prelength, postlength):
    if syn_spec is None:
        return syn_spec
    rule = conn_spec['rule']
    if isinstance(syn_spec, str):
        return kernel.SLILiteral(syn_spec)
    elif isinstance(syn_spec, dict):
        for key, value in syn_spec.items():
            # if value is a list, it is converted to a numpy array
            if isinstance(value, (list, tuple)):
                value = numpy.asarray(value)

            if isinstance(value, (numpy.ndarray, numpy.generic)):
                if len(value.shape) == 1:
                    if rule == 'one_to_one':
                        if value.shape[0] != prelength:
                            raise kernel.NESTError(
                                "'" + key + "' has to be an array of "
                                "dimension " + str(prelength) + ", a "
                                "scalar or a dictionary.")
                        else:
                            syn_spec[key] = value
                    elif rule == 'fixed_total_number':
                        if ('N' in conn_spec and value.shape[0] != conn_spec['N']):
                            raise kernel.NESTError(
                                "'" + key + "' has to be an array of "
                                "dimension " + str(conn_spec['N']) + ", a "
                                "scalar or a dictionary.")
                    else:
                        raise kernel.NESTError(
                            "'" + key + "' has the wrong type. "
                            "One-dimensional parameter arrays can "
                            "only be used in conjunction with rule "
                            "'one_to_one' or 'fixed_total_number'.")

                elif len(value.shape) == 2:
                    if rule == 'all_to_all':
                        if value.shape[0] != postlength or \
                                value.shape[1] != prelength:

                            raise kernel.NESTError(
                                "'" + key + "' has to be an array of "
                                "dimension " + str(postlength) + "x" +
                                str(prelength) +
                                " (n_target x n_sources), " +
                                "a scalar or a dictionary.")
                        else:
                            syn_spec[key] = value.flatten()
                    elif rule == 'fixed_indegree':
                        indegree = conn_spec['indegree']
                        if value.shape[0] != postlength or \
                                value.shape[1] != indegree:
                            raise kernel.NESTError(
                                "'" + key + "' has to be an array of "
                                "dimension " + str(postlength) + "x" +
                                str(indegree) +
                                " (n_target x indegree), " +
                                "a scalar or a dictionary.")
                        else:
                            syn_spec[key] = value.flatten()
                    elif rule == 'fixed_outdegree':
                        outdegree = conn_spec['outdegree']
                        if value.shape[0] != prelength or \
                                value.shape[1] != outdegree:
                            raise kernel.NESTError(
                                "'" + key + "' has to be an array of "
                                "dimension " + str(prelength) + "x" +
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
        return syn_spec
    else:
        raise TypeError("syn_spec must be a string or dict")


def _process_spatial_projections(conn_spec, syn_spec):
    allowed_conn_spec_keys = ['mask', 'allow_multapses', 'allow_autapses', 'rule',
                              'indegree', 'outdegree', 'p', 'use_on_source', 'allow_oversized_mask']
    allowed_syn_spec_keys = ['weight', 'delay', 'synapse_model']
    for key in conn_spec.keys():
        if key not in allowed_conn_spec_keys:
            raise ValueError(
                "'{}' is not allowed in conn_spec when".format(key) +
                " connecting with mask or kernel")

    projections = {}
    projections.update(conn_spec)
    if 'p' in conn_spec:
        projections['kernel'] = projections.pop('p')
    if syn_spec is not None:
        for key in syn_spec.keys():
            if key not in allowed_syn_spec_keys:
                raise ValueError(
                    "'{}' is not allowed in syn_spec when ".format(key) +
                    "connecting with mask or kernel".format(key))
        projections.update(syn_spec)

    if conn_spec['rule'] == 'fixed_indegree':
        if 'use_on_source' in conn_spec:
            raise ValueError(
                "'use_on_source' can only be set when using " +
                "pairwise_bernoulli")
        projections['connection_type'] = 'pairwise_bernoulli_on_source'
        projections['number_of_connections'] = projections.pop('indegree')
    elif conn_spec['rule'] == 'fixed_outdegree':
        if 'use_on_source' in conn_spec:
            raise ValueError(
                "'use_on_source' can only be set when using " +
                "pairwise_bernoulli")
        projections['connection_type'] = 'pairwise_bernoulli_on_target'
        projections['number_of_connections'] = projections.pop('outdegree')
    elif conn_spec['rule'] == 'pairwise_bernoulli':
        if ('use_on_source' in conn_spec and
                conn_spec['use_on_source']):
            projections['connection_type'] = 'pairwise_bernoulli_on_source'
            projections.pop('use_on_source')
        else:
            projections['connection_type'] = 'pairwise_bernoulli_on_target'
            if 'use_on_source' in projections:
                projections.pop('use_on_source')
    else:
        raise kernel.NESTError("When using kernel or mask, the only possible "
                               "connection rules are 'pairwise_bernoulli', "
                               "'fixed_indegree', or 'fixed_outdegree'")
    projections.pop('rule')
    return projections


def _connect_layers_needed(conn_spec, syn_spec):
    if isinstance(conn_spec, dict):
        # If a conn_spec entry is based on spatial properties, we must use ConnectLayers.
        for key, item in conn_spec.items():
            if isinstance(item, Parameter) and item.is_spatial():
                return True
        # We must use ConnectLayers in some additional cases.
        rule_is_bernoulli = 'pairwise_bernoulli' in str(conn_spec['rule'])
        if ('mask' in conn_spec or
                ('p' in conn_spec and not rule_is_bernoulli) or
                'use_on_source' in conn_spec):
            return True
    # If a syn_spec entry is based on spatial properties, we must use ConnectLayers.
    if isinstance(syn_spec, dict):
        for key, item in syn_spec.items():
            if isinstance(item, Parameter) and item.is_spatial():
                return True
    # If we get here, there is not need to use ConnectLayers.
    return False


def _connect_spatial(pre, post, projections):
    # Replace python classes with SLI datums
    def fixdict(d):
        d = d.copy()
        for k, v in d.items():
            if isinstance(v, dict):
                d[k] = fixdict(v)
            elif isinstance(v, Mask) or isinstance(v, Parameter):
                d[k] = v._datum
        return d

    projections = fixdict(projections)
    sps(projections)
    sr('ConnectLayers')


def _connect_nonunique(syn_spec):
    sps({} if syn_spec is None else syn_spec)
    sli_run('Connect_nonunique')


@check_stack
def Connect(pre, post, conn_spec=None, syn_spec=None,
            return_connectome=False):
    """
    Connect pre nodes to post nodes.

    Nodes in pre and post are connected using the specified connectivity
    (all-to-all by default) and synapse type (static_synapse by default).
    Details depend on the connectivity rule.

    Parameters
    ----------
    pre : GIDCollection
        Presynaptic nodes, as object representing the global IDs of the nodes
    post : GIDCollection
        Postsynaptic nodes, as object representing the global IDs of the nodes
    conn_spec : str or dict, optional
        Specifies connectivity rule, see below
    syn_spec : str or dict, optional
        Specifies synapse model, see below
    return_connectome: bool
        Specifies whether or not we should return a Connectome of pre and post connections

    Raises
    ------
    kernel.NESTError

    Notes
    -----
    It is possible to connect arrays of GIDs with nonunique GIDs by
    passing the arrays as pre and post, together with a syn_spec dictionary.
    However this should only be done if you know what you're doing. This will
    connect all nodes in pre to all nodes in post and apply the specified
    synapse specifications.

    Connectivity specification (conn_spec)
    --------------------------------------

    Connectivity is specified either as a string containing the name of a
    connectivity rule (default: 'all_to_all') or as a dictionary specifying
    the rule and any mandatory rule-specific parameters (e.g. 'indegree').

    In addition, switches setting permission for establishing
    self-connections ('allow_autapses', default: True) and multiple connections
    between a pair of nodes ('allow_multapses', default: True) can be contained
    in the dictionary. Another switch enables the creation of symmetric
    connections ('make_symmetric', default: False) by also creating connections
    in the opposite direction.

    If pre and post have spatial posistions, a `mask` can be specified as a dictionary. The mask define which
    nodes are considered as potential targets for each source node. Connection with spatial nodes can also
    use nest.distribution as parameters, for instance for the probability `p`.

    Available rules and associated parameters
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    - 'all_to_all' (default)
    - 'one_to_one'
    - 'fixed_indegree', 'indegree'
    - 'fixed_outdegree', 'outdegree'
    - 'fixed_total_number', 'N'
    - 'pairwise_bernoulli', 'p'
    - 'symmetric_pairwise_bernoulli', 'p'

    Example conn-spec choices
    ~~~~~~~~~~~~~~~~~~~~~~~~~
    - 'one_to_one'
    - {'rule': 'fixed_indegree', 'indegree': 2500, 'allow_autapses': False}
    - {'rule': 'pairwise_bernoulli', 'p': 0.1}
    - {'rule': 'pairwise_bernoulli', 'p': nest.distribution.exponential(nest.spatial.distance),
       'mask': {'rectangular': {'lower_left'  : [-2.0, -1.0],
                                'upper_right' : [ 2.0,  1.0]}}}

    Synapse specification (syn_spec)
    --------------------------------------

    The synapse model and its properties can be given either as a string
    identifying a specific synapse model (default: 'static_synapse') or
    as a dictionary specifying the synapse model and its parameters.

    Available keys in the synapse specification dictionary are:
    - 'synapse_model'
    - 'weight'
    - 'delay'
    - 'receptor_type'
    - any parameters specific to the selected synapse model.

    All parameters are optional and if not specified, the default values
    of the synapse model will be used. The key 'synapse_model' identifies the
    synapse model, this can be one of NEST's built-in synapse models
    or a user-defined model created via CopyModel().

    If 'synapse_model' is not specified the default model 'static_synapse'
    will be used.

    All other parameters can be scalars, arrays, nest.Parameter or distributions.
    In the case of scalar parameters, all keys must be doubles
    except for 'receptor_type' which must be initialised with an integer.

    Parameter arrays are available for the rules 'one_to_one',
    'all_to_all', 'fixed_total_number', 'fixed_indegree' and
    'fixed_outdegree':
    - For 'one_to_one' the array has to be a one-dimensional
      NumPy array with length len(pre).
    - For 'all_to_all' the array has to be a two-dimensional NumPy array
      with shape (len(post), len(pre)), therefore the rows describe the
      target and the columns the source neurons.
    - For 'fixed_total_number' the array has to be a one-dimensional
      NumPy array with length len(N), where N is the number of connections
      specified.
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

    Distributed parameters can be defined through NEST's different parametertypes. NEST has various
    random parameters, spatial parameters and distributions (only accesseable for nodes with spatial positions),
    logical expressions and mathematical expressions, which can be used to define node and connection parameters.

    To see all available parameters, see documentation defined in distributions, logic, math,
    random and spatial modules.

    Example NEST parametertypes
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~
    - nest.random.uniform(min, max)
    - nest.random.normal(mean, std)
    - nest.math.cos(nest.Parameter)
    - nest.spatial.distance
    - nest.distribution.exponential(nest.Parameter, beta)

    Distributed parameters can also be initialised with a dictionary
    specifying the distribution type ('distribution', e.g. 'normal') and
    any distribution-specific parameters (e.g. 'mean' and 'std').

    To see all available distributions, run:
    nest.ll_api.sli_run('rdevdict info')

    To get information on a particular distribution, e.g. 'binomial', run:
    nest.help('rdevdict::binomial')

    Most common available distributions and associated parameters
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    - 'normal' with 'mean', 'std'
    - 'lognormal' with 'mean', 'std'
    - 'exponential' with 'beta'
    - 'uniform' with 'low', 'high'

    Example syn-spec choices
    ~~~~~~~~~~~~~~~~~~~~~~~~~
    - 'stdp_synapse'
    - {'weight': 2.4, 'receptor_type': 1}
    - {'synapse_model': 'stdp_synapse',
       'weight': 2.5,
       'delay': nest.random.uniform(0.8, 2.5),
       'alpha': nest.random.normal(5.0, 1.0)
      }
    """

    # Converting conn_spec to dict, without putting it on the SLI stack.
    processed_conn_spec = _process_conn_spec(conn_spec)
    # If syn_spec is given, its contents are checked, and if needed converted
    # to the right formats.
    processed_syn_spec = _process_syn_spec(
        syn_spec, processed_conn_spec, len(pre), len(post))

    pre_is_array_of_gids = isinstance(pre, (list, tuple, numpy.ndarray))
    post_is_array_of_gids = isinstance(post, (list, tuple, numpy.ndarray))
    # If pre and post are arrays of GIDs and no conn_spec is specified,
    # the GIDs are connected all_to_all. If the arrays contain unique
    # GIDs, a warning is issued.
    if pre_is_array_of_gids and post_is_array_of_gids and conn_spec is None:
        if return_connectome:
            raise ValueError("Connectome cannot be returned when connecting two arrays of GIDs")
        if len(numpy.unique(pre)) == len(pre) and len(numpy.unique(post)) == len(post):
            warnings.warn('Connecting two arrays of GIDs should only be done in cases where one or both the arrays '
                          'contain non-unique GIDs. Use GIDCollections when connecting two collections of unique GIDs.')
        # Connect_nonunique doesn't support connecting numpy arrays
        sps(list(pre))
        sps(list(post))
        _connect_nonunique(processed_syn_spec)
        return

    sps(pre)
    sps(post)

    if not isinstance(pre, GIDCollection):
        raise TypeError("Not implemented, presynaptic nodes must be a "
                        "GIDCollection")
    if not isinstance(post, GIDCollection):
        raise TypeError("Not implemented, postsynaptic nodes must be a "
                        "GIDCollection")

    # In some cases we must connect with ConnectLayers instead.
    if _connect_layers_needed(processed_conn_spec, processed_syn_spec):
        # Check that pre and post are layers
        if pre.spatial is None:
            raise TypeError(
                "Presynaptic GIDCollection must have spatial information")
        if post.spatial is None:
            raise TypeError(
                "Presynaptic GIDCollection must have spatial information")

        # Create the projection dictionary
        spatial_projections = _process_spatial_projections(
            processed_conn_spec, processed_syn_spec)

        # Connect using ConnectLayers
        _connect_spatial(pre, post, spatial_projections)
    else:
        sps(processed_conn_spec)
        if processed_syn_spec is not None:
            sps(processed_syn_spec)
        sr('Connect')

    if return_connectome:
        return GetConnections(pre, post)


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
      https://www.nest-simulator.org/connection-generator-interface
    * The GitHub repository and documentation for libneurosim at
      https://github.com/INCF/libneurosim/
    * The publication about the Connection Generator Interface at
      https://doi.org/10.3389/fninf.2014.00043

    Parameters
    ----------
    pre : GIDCollection
        GIDs of presynaptic nodes
    post : GIDCollection
        GIDs of postsynaptic nodes
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

    sli_func('CGConnect', cg, pre, post, parameter_map, '/' + model,
             litconv=True)


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
def Disconnect(pre, post, conn_spec='one_to_one', syn_spec='static_synapse'):
    """Disconnect pre neurons from post neurons.

    Neurons in pre and post are disconnected using the specified disconnection
    rule (one-to-one by default) and synapse type (static_synapse by default).
    Details depend on the disconnection rule.

    Parameters
    ----------
    pre : GIDCollection
        Presynaptic nodes, given as GIDCollection
    post : GIDCollection
        Postsynaptic nodes, given as GIDCollection
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
    syn_spec is given as a non-empty dictionary, the 'synapse_model' parameter must be
    present.

    If no syn_spec is specified the default model 'static_synapse' will be used.

    Available keys in the synapse dictionary are:
    - 'synapse_model'
    - 'weight'
    - 'delay',
    - 'receptor_type'
    - parameters specific to the synapse model chosen

    All parameters except synapse_model are optional and if not specified will use the default
    values determined by the current synapse model.

    'synapse_model' determines the synapse type, taken from pre-defined synapse
    types in NEST or manually specified synapses created via CopyModel().

    All other parameters are not currently implemented.

    Notes
    -----
    Disconnect only disconnects explicitly specified nodes.
    """

    sps(pre)
    sps(post)

    if is_string(conn_spec):
        conn_spec = {'rule': conn_spec}
    if is_string(syn_spec):
        syn_spec = {'synapse_model': syn_spec}

    sps(conn_spec)
    sps(syn_spec)

    sr('Disconnect_g_g_D_D')
