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
from .hl_api_info import GetStatus
from .hl_api_nodes import Create
from .hl_api_parallel_computing import NumProcesses
from .hl_api_projections import BuildNetwork
from .hl_api_types import NodeCollection, SynapseCollection, Mask, Parameter

__all__ = [
    "_array_connect",
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

    BuildNetwork()

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
def _array_connect(pre, post, conn_spec, syn_spec=None):
    """
    Connect `pre` nodes to `post` nodes with one-to-one scheme.

    `pre` and `post` are arrays of node IDs, which might contain non-unique IDs. The
    arrays must be of the same length. You may also specify weight, delay, and
    receptor type for each connection as NumPy arrays in the `syn_spec` dictionary.

    Lists of synapse models and connection rules are available as
    ``nest.synapse_models`` and ``nest.connection_rules``, respectively.

    Parameters
    ----------
    pre: list/array/NodeCollection
        Node IDs
    post: list/array/NodeCollection
        Node IDs
    conn_spec: Dictionary
        Connection specifications. Must contain the keyword `rule`
    syn_spec: Dictionary (optional)
        Synapse specifications
        If `weight`, `delay`, or `receptor_type` is specified, they must be given as
        single elements or as arrays with the length of pre and post.
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

    return use_connect_arrays


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
