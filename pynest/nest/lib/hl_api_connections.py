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

from .. import pynestkernel as kernel
from ..ll_api import check_stack, connect_arrays, spp, sps, sr
from .hl_api_connection_helpers import (
    _connect_layers_needed,
    _connect_spatial,
    _process_conn_spec,
    _process_input_nodes,
    _process_spatial_projections,
    _process_syn_spec,
)
from .hl_api_helper import is_string
from .hl_api_types import CollocatedSynapses, NodeCollection, SynapseCollection

__all__ = [
    "Connect",
    "TripartiteConnect",
    "Disconnect",
    "GetConnections",
]


@check_stack
def GetConnections(source=None, target=None, synapse_model=None, synapse_label=None):
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
            params["source"] = source
        else:
            raise TypeError("source must be NodeCollection.")

    if target is not None:
        if isinstance(target, NodeCollection):
            params["target"] = target
        else:
            raise TypeError("target must be NodeCollection.")

    if synapse_model is not None:
        params["synapse_model"] = kernel.SLILiteral(synapse_model)

    if synapse_label is not None:
        params["synapse_label"] = synapse_label

    sps(params)
    sr("GetConnections")

    conns = spp()

    if isinstance(conns, tuple):
        conns = SynapseCollection(None)

    return conns


@check_stack
def Connect(pre, post, conn_spec=None, syn_spec=None, return_synapsecollection=False):
    """
    Connect `pre` nodes to `post` nodes.

    Nodes in `pre` and `post` are connected using the specified connectivity
    (`all-to-all` by default) and synapse type (:cpp:class:`static_synapse <nest::static_synapse>` by default).
    Details depend on the connectivity rule.

    Lists of synapse models and connection rules are available as
    ``nest.synapse_models`` and ``nest.connection_rules``, respectively.

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
    identifying a specific synapse model (default: :cpp:class:`static_synapse <nest::static_synapse>`) or
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

    If `synapse_model` is not specified the default model :cpp:class:`static_synapse <nest::static_synapse>`
    will be used.

    Distributed parameters can be defined through NEST's different parametertypes. NEST has various
    random parameters, spatial parameters and distributions (only accessible for nodes with spatial positions),
    logical expressions and mathematical expressions, which can be used to define node and connection parameters.

    To see all available parameters, see documentation defined in distributions, logic, math,
    random and spatial modules.

    See Also
    ---------
    :ref:`connection_management`
    """

    use_connect_arrays, pre, post = _process_input_nodes(pre, post, conn_spec)

    # Converting conn_spec to dict, without putting it on the SLI stack.
    processed_conn_spec = _process_conn_spec(conn_spec)
    # If syn_spec is given, its contents are checked, and if needed converted
    # to the right formats.
    processed_syn_spec = _process_syn_spec(syn_spec, processed_conn_spec, len(pre), len(post), use_connect_arrays)

    # If pre and post are arrays of node IDs, and conn_spec is unspecified,
    # the node IDs are connected one-to-one.
    if use_connect_arrays:
        if return_synapsecollection:
            raise ValueError("SynapseCollection cannot be returned when connecting two arrays of node IDs")

        if processed_syn_spec is None:
            raise ValueError(
                "When connecting two arrays of node IDs, the synapse specification dictionary must "
                "be specified and contain at least the synapse model."
            )

        # In case of misspelling
        if "weights" in processed_syn_spec:
            raise ValueError("To specify weights, use 'weight' in syn_spec.")
        if "delays" in processed_syn_spec:
            raise ValueError("To specify delays, use 'delay' in syn_spec.")

        weights = numpy.array(processed_syn_spec["weight"]) if "weight" in processed_syn_spec else None
        delays = numpy.array(processed_syn_spec["delay"]) if "delay" in processed_syn_spec else None

        try:
            synapse_model = processed_syn_spec["synapse_model"]
        except KeyError:
            raise ValueError(
                "When connecting two arrays of node IDs, the synapse specification dictionary must "
                "contain a synapse model."
            )

        # Split remaining syn_spec entries to key and value arrays
        reduced_processed_syn_spec = {
            k: processed_syn_spec[k]
            for k in set(processed_syn_spec.keys()).difference(set(("weight", "delay", "synapse_model")))
        }

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
        spatial_projections = _process_spatial_projections(processed_conn_spec, processed_syn_spec)

        # Connect using ConnectLayers
        _connect_spatial(pre, post, spatial_projections)
    else:
        sps(processed_conn_spec)
        if processed_syn_spec is not None:
            sps(processed_syn_spec)
        sr("Connect")

    if return_synapsecollection:
        return GetConnections(pre, post)


@check_stack
def TripartiteConnect(pre, post, third, conn_spec, syn_specs=None):
    """
    Connect `pre` nodes to `post` nodes and a `third`-factor nodes.

    Nodes in `pre` and `post` are connected using the specified tripartite connection rule
    and the given synapse types (all :cpp:class:`static_synapse <nest::static_synapse>` by default).
    Details depend on the connection rule.

    Lists of synapse models and connection rules are available as
    ``nest.synapse_models`` and ``nest.connection_rules``, respectively. Note that only tripartite
    connection rules can be used.

    Parameters
    ----------
    pre : NodeCollection
        Presynaptic nodes
    post : NodeCollection
        Postsynaptic nodes
    third : NodeCollection
        Third population to include in connection
    conn_spec : dict
        Specifies connection rule, which must support tripartite connections, see below
    syn_spec : dict, optional
        Specifies synapse models to be used, see below

    Raises
    ------
    kernel.NESTError

    Notes
    -----
    **Connectivity specification (conn_spec)**

    Available tripartite rules::

     - ``tripartite_bernoulli_with_pool``

    See :ref:`tripartite_connectivity` for more details and :doc:`/auto_examples/astrocytes/astrocyte_small_network`
    and :doc:`/auto_examples/astrocytes/astrocyte_brunel` for examples.

    **Synapse specifications (syn_specs)**

    Synapse specifications for tripartite connections are given as a dictionary with specifications
    for each of the three projections to be created::

     {"primary": <syn_spec>,
     "third_in": <syn_spec>,
     "third_out": <syn_spec>}

    Here, ``"primary"`` marks the synapse specification for the projections between ``pre`` and ``post`` nodes,
    ``"third_in"`` for connections between ``pre`` and ``third`` nodes and ``"third_out"`` for connections between
    ``third`` and ``post`` nodes.

    Each ``<syn_spec>`` entry can be any entry that would be possible as synapse specification
    in a normal ``Connect()`` call. Any missing entries default to ``static_synapse``. If no ``<syn_spec>`` argument
    is given at all, all three entries default to ``static_synapse``.

    The synapse model and its properties can be given either as a string
    identifying a specific synapse model (default: :cpp:class:`static_synapse <nest::static_synapse>`) or
    as a dictionary specifying the synapse model and its parameters.

    Available keys in the synapse specification dictionary are::

     - 'synapse_model'
     - 'weight'
     - 'delay'
     - 'receptor_type'
     - any parameters specific to the selected synapse model.


    See Also
    ---------
    :ref:`connection_management`
    """

    # Confirm that we got node collections
    if not isinstance(pre, NodeCollection):
        raise TypeError("Presynaptic nodes must be a NodeCollection")
    if not isinstance(post, NodeCollection):
        raise TypeError("Postsynaptic nodes must be a NodeCollection")
    if not isinstance(third, NodeCollection):
        raise TypeError("Third-factor nodes must be a NodeCollection")

    # Normalize syn_specs: ensure all three entries are in place, are collocated synapses and do not contain lists
    syn_specs = syn_specs if syn_specs is not None else dict()
    SYN_KEYS = {"primary", "third_in", "third_out"}
    for key in SYN_KEYS:
        if key not in syn_specs:
            syn_specs[key] = {"synapse_model": "static_synapse"}
        elif isinstance(syn_specs[key], str):
            syn_specs[key] = {"synapse_model": syn_specs[key]}

        if not isinstance(syn_specs[key], CollocatedSynapses):
            syn_specs[key] = CollocatedSynapses(syn_specs[key])

        for synspec in syn_specs[key].syn_specs:
            for entry, value in synspec.items():
                if isinstance(value, (list, tuple, numpy.ndarray)):
                    raise ValueError(
                        f"Tripartite connections do not accept parameter lists,"
                        f"but 'syn_specs[{key}][{entry}]' is a list or similar."
                    )

    sps(pre)
    sps(post)
    sps(third)
    sps(conn_spec)
    sps(syn_specs)
    sr("ConnectTripartite_g_g_g_D_D")


@check_stack
def Disconnect(*args, conn_spec=None, syn_spec=None):
    """Disconnect connections in a SynapseCollection, or `pre` neurons from `post` neurons.

    When specifying `pre` and `post` nodes, they are disconnected using the specified disconnection
    rule (one-to-one by default) and synapse type (:cpp:class:`static_synapse <nest::static_synapse>` by default).
    Details depend on the disconnection rule.

    Parameters
    ----------
    args : SynapseCollection or NodeCollections
        Either a collection of connections to disconnect, or pre- and postsynaptic nodes given as NodeCollections
    conn_spec : str or dict
        Disconnection rule when specifying pre- and postsynaptic nodes, see below
    syn_spec : str or dict
        Synapse specifications when specifying pre- and postsynaptic nodes, see below

    Notes
    -------

    **conn_spec**

    Apply the same rules as for connectivity specs in the :py:func:`.Connect` method

    Possible choices of the conn_spec are

    - 'one_to_one'
    - 'all_to_all'

    **syn_spec**

    The synapse model and its properties can be specified either as a string naming
    a synapse model (the list of all available synapse models can be gotten via
    ``nest.synapse_models``) or as a dictionary as described below.

    Note that only the synapse type is checked when we disconnect and that if
    `syn_spec` is given as a non-empty dictionary, the 'synapse_model' parameter must
    be present.

    If no synapse model is specified the default model
    :cpp:class:`static_synapse <nest::static_synapse>` will be used.

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

    if len(args) == 1:
        synapsecollection = args[0]
        if not isinstance(synapsecollection, SynapseCollection):
            raise TypeError("Arguments must be either a SynapseCollection or two NodeCollections")
        if conn_spec is not None or syn_spec is not None:
            raise ValueError("When disconnecting with a SynapseCollection, conn_spec and syn_spec cannot be specified")
        synapsecollection.disconnect()
    elif len(args) == 2:
        # Fill default values
        conn_spec = "one_to_one" if conn_spec is None else conn_spec
        syn_spec = "static_synapse" if syn_spec is None else syn_spec
        if is_string(conn_spec):
            conn_spec = {"rule": conn_spec}
        if is_string(syn_spec):
            syn_spec = {"synapse_model": syn_spec}
        pre, post = args
        if not isinstance(pre, NodeCollection) or not isinstance(post, NodeCollection):
            raise TypeError("Arguments must be either a SynapseCollection or two NodeCollections")
        sps(pre)
        sps(post)
        sps(conn_spec)
        sps(syn_spec)
        sr("Disconnect_g_g_D_D")
    else:
        raise TypeError("Arguments must be either a SynapseCollection or two NodeCollections")
