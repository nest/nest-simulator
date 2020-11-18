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

from ..ll_api import *
from .. import pynestkernel as kernel

from .hl_api_connection_helpers import (_process_input_nodes, _connect_layers_needed,
                                        _connect_spatial, _process_conn_spec,
                                        _process_spatial_projections, _process_syn_spec)
from .hl_api_helper import *
from .hl_api_info import GetStatus
from .hl_api_nodes import Create
from .hl_api_parallel_computing import NumProcesses
from .hl_api_simulation import GetKernelStatus, SetKernelStatus
from .hl_api_types import NodeCollection, SynapseCollection, Mask, Parameter

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
    """Return a `SynapseCollection` representing the connection identifiers.

    Any combination of `source`, `target`, `synapse_model` and
    `synapse_label` parameters is permitted.

    Parameters
    ----------
    source : NodeCollection, optional
        Source node IDs, only connections from these
        pre-synaptic neurons are returned
    target : NodeCollection, optional
        Target node IDs, only connections to these
        postsynaptic neurons are returned
    synapse_model : str, optional
        Only connections with this synapse type are returned
    synapse_label : int, optional
        (non-negative) only connections with this synapse label are returned

    Returns
    -------
    SynapseCollection:
        Object representing the source-node_id, target-node_id, target-thread, synapse-id, port of connections, see
        :py:class:`.SynapseCollection` for more.

    Raises
    ------
    TypeError

    Notes
    -----
    Only connections with targets on the MPI process executing
    the command are returned.
    """

    params = {}

    if source is not None:
        if isinstance(source, NodeCollection):
            params['source'] = source
        else:
            raise TypeError("source must be NodeCollection.")

    if target is not None:
        if isinstance(target, NodeCollection):
            params['target'] = target
        else:
            raise TypeError("target must be NodeCollection.")

    if synapse_model is not None:
        params['synapse_model'] = kernel.SLILiteral(synapse_model)

    if synapse_label is not None:
        params['synapse_label'] = synapse_label

    sps(params)
    sr("GetConnections")

    conns = spp()

    if isinstance(conns, tuple):
        conns = SynapseCollection(None)

    return conns


@check_stack
def Connect(pre, post, conn_spec=None, syn_spec=None,
            return_synapsecollection=False):
    """
    Connect `pre` nodes to `post` nodes.

    Nodes in `pre` and `post` are connected using the specified connectivity
    (`all-to-all` by default) and synapse type (:cpp:class:`static_synapse <nest::StaticConnection>` by default).
    Details depend on the connectivity rule.

    Parameters
    ----------
    pre : NodeCollection (or array-like object)
        Presynaptic nodes, as object representing the IDs of the nodes
    post : NodeCollection (or array-like object)
        Postsynaptic nodes, as object representing the IDs of the nodes
    conn_spec : str or dict, optional
        Specifies connectivity rule, see below
    syn_spec : str or dict, optional
        Specifies synapse model, see below
    return_synapsecollection: bool
        Specifies whether or not we should return a :py:class:`.SynapseCollection` of pre and post connections

    Raises
    ------
    kernel.NESTError

    Notes
    -----
    It is possible to connect NumPy arrays of node IDs one-to-one by passing the arrays as `pre` and `post`,
    specifying `'one_to_one'` for `conn_spec`.
    In that case, the arrays may contain non-unique IDs.
    You may also specify weight, delay, and receptor type for each connection as NumPy arrays in the `syn_spec`
    dictionary.
    This feature is currently not available when MPI is used; trying to connect arrays with more than one
    MPI process will raise an error.

    If pre and post have spatial positions, a `mask` can be specified as a dictionary. The mask define which
    nodes are considered as potential targets for each source node. Connections with spatial nodes can also
    use `nest.spatial_distributions` as parameters, for instance for the probability `p`.

    **Connectivity specification (conn_spec)**

    Available rules and associated parameters::

     - 'all_to_all' (default)
     - 'one_to_one'
     - 'fixed_indegree', 'indegree'
     - 'fixed_outdegree', 'outdegree'
     - 'fixed_total_number', 'N'
     - 'pairwise_bernoulli', 'p'
     - 'symmetric_pairwise_bernoulli', 'p'

    See :ref:`conn_rules` for more details, including example usage.

    **Synapse specification (syn_spec)**

    The synapse model and its properties can be given either as a string
    identifying a specific synapse model (default: :cpp:class:`static_synapse <nest::StaticConnection>`) or
    as a dictionary specifying the synapse model and its parameters.

    Available keys in the synapse specification dictionary are::

     - 'synapse_model'
     - 'weight'
     - 'delay'
     - 'receptor_type'
     - any parameters specific to the selected synapse model.

    See :ref:`synapse_spec` for details, including example usage.

    All parameters are optional and if not specified, the default values
    of the synapse model will be used. The key 'synapse_model' identifies the
    synapse model, this can be one of NEST's built-in synapse models
    or a user-defined model created via :py:func:`.CopyModel`.

    If `synapse_model` is not specified the default model :cpp:class:`static_synapse <nest::StaticConnection>`
    will be used.

    Distributed parameters can be defined through NEST's different parametertypes. NEST has various
    random parameters, spatial parameters and distributions (only accesseable for nodes with spatial positions),
    logical expressions and mathematical expressions, which can be used to define node and connection parameters.

    To see all available parameters, see documentation defined in distributions, logic, math,
    random and spatial modules.

    See Also
    ---------
    :ref:`connection_mgnt`
    """
    use_connect_arrays, pre, post = _process_input_nodes(pre, post, conn_spec)

    # Converting conn_spec to dict, without putting it on the SLI stack.
    processed_conn_spec = _process_conn_spec(conn_spec)
    # If syn_spec is given, its contents are checked, and if needed converted
    # to the right formats.
    processed_syn_spec = _process_syn_spec(
        syn_spec, processed_conn_spec, len(pre), len(post), use_connect_arrays)

    # If pre and post are arrays of node IDs, and conn_spec is unspecified,
    # the node IDs are connected one-to-one.
    if use_connect_arrays:
        if return_synapsecollection:
            raise ValueError("SynapseCollection cannot be returned when connecting two arrays of node IDs")

        if processed_syn_spec is None:
            raise ValueError("When connecting two arrays of node IDs, the synapse specification dictionary must "
                             "be specified and contain at least the synapse model.")

        weights = numpy.array(processed_syn_spec['weight']) if 'weight' in processed_syn_spec else None
        delays = numpy.array(processed_syn_spec['delay']) if 'delay' in processed_syn_spec else None

        try:
            synapse_model = processed_syn_spec['synapse_model']
        except KeyError:
            raise ValueError("When connecting two arrays of node IDs, the synapse specification dictionary must "
                             "contain a synapse model.")

        # Split remaining syn_spec entries to key and value arrays
        reduced_processed_syn_spec = {k: processed_syn_spec[k]
                                      for k in set(processed_syn_spec.keys()).difference(
                                          set(('weight', 'delay', 'synapse_model')))}

        if len(reduced_processed_syn_spec) > 0:
            syn_param_keys = numpy.array(list(reduced_processed_syn_spec.keys()), dtype=numpy.string_)
            syn_param_values = numpy.zeros([len(reduced_processed_syn_spec), len(pre)])

            for i, value in enumerate(reduced_processed_syn_spec.values()):
                syn_param_values[i] = value
        else:
            syn_param_keys = None
            syn_param_values = None

        connect_arrays(pre, post, weights, delays, synapse_model, syn_param_keys, syn_param_values)

        return

    sps(pre)
    sps(post)

    if not isinstance(pre, NodeCollection):
        raise TypeError("Not implemented, presynaptic nodes must be a NodeCollection")
    if not isinstance(post, NodeCollection):
        raise TypeError("Not implemented, postsynaptic nodes must be a NodeCollection")

    # In some cases we must connect with ConnectLayers instead.
    if _connect_layers_needed(processed_conn_spec, processed_syn_spec):
        # Check that pre and post are layers
        if pre.spatial is None:
            raise TypeError("Presynaptic NodeCollection must have spatial information")
        if post.spatial is None:
            raise TypeError("Presynaptic NodeCollection must have spatial information")

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

    if return_synapsecollection:
        return GetConnections(pre, post)


@check_stack
def CGConnect(pre, post, cg, parameter_map=None, model="static_synapse"):
    """Connect neurons using the Connection Generator Interface.

    Potential pre-synaptic neurons are taken from `pre`, potential
    postsynaptic neurons are taken from `post`. The connection
    generator `cg` specifies the exact connectivity to be set up. The
    `parameter_map` can either be None or a dictionary that maps the
    keys `weight` and `delay` to their integer indices in the value
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
    pre : NodeCollection
        node IDs of presynaptic nodes
    post : NodeCollection
        node IDs of postsynaptic nodes
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
        raise kernel.NESTError("NEST was not compiled with support for libneurosim: CGConnect is not available.")

    if parameter_map is None:
        parameter_map = {}

    sli_func('CGConnect', cg, pre, post, parameter_map, '/' + model,
             litconv=True)


@check_stack
def CGParse(xml_filename):
    """Parse an XML file and return the corresponding connection
    generator cg.

    The library to provide the parsing can be selected
    by :py:func:`.CGSelectImplementation`.

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
        raise kernel.NESTError("NEST was not compiled with support for libneurosim: CGParse is not available.")

    sps(xml_filename)
    sr("CGParse")
    return spp()


@check_stack
def CGSelectImplementation(tag, library):
    """Select a library to provide a parser for XML files and associate
    an XML tag with the library.

    XML files can be read by :py:func:`.CGParse`.

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
            "NEST was not compiled with support for libneurosim: CGSelectImplementation is not available.")

    sps(tag)
    sps(library)
    sr("CGSelectImplementation")


@check_stack
def Disconnect(pre, post, conn_spec='one_to_one', syn_spec='static_synapse'):
    """Disconnect `pre` neurons from `post` neurons.

    Neurons in `pre` and `post` are disconnected using the specified disconnection
    rule (one-to-one by default) and synapse type (:cpp:class:`static_synapse <nest::StaticConnection>` by default).
    Details depend on the disconnection rule.

    Parameters
    ----------
    pre : NodeCollection
        Presynaptic nodes, given as `NodeCollection`
    post : NodeCollection
        Postsynaptic nodes, given as `NodeCollection`
    conn_spec : str or dict
        Disconnection rule, see below
    syn_spec : str or dict
        Synapse specifications, see below

    Notes
    -------

    **conn_spec**

    Apply the same rules as for connectivity specs in the :py:func:`.Connect` method

    Possible choices of the conn_spec are
    ::
     - 'one_to_one'
     - 'all_to_all'

    **syn_spec**

    The synapse model and its properties can be inserted either as a
    string describing one synapse model (synapse models are listed in the
    synapsedict) or as a dictionary as described below.

    Note that only the synapse type is checked when we disconnect and that if
    `syn_spec` is given as a non-empty dictionary, the 'synapse_model' parameter must be
    present.

    If no synapse model is specified the default model :cpp:class:`static_synapse <nest::StaticConnection>`
    will be used.

    Available keys in the synapse dictionary are:
    ::

    - 'synapse_model'
    - 'weight'
    - 'delay',
    - 'receptor_type'
    - parameters specific to the synapse model chosen

    'synapse_model' determines the synapse type, taken from pre-defined synapse
    types in NEST or manually specified synapses created via :py:func:`.CopyModel`.

    All other parameters are not currently implemented.

    Notes
    -----
    `Disconnect` only disconnects explicitly specified nodes.
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
