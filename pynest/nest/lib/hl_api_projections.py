# -*- coding: utf-8 -*-
#
# hl_api_projections.py
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
Connection projection functions and classes
"""

import copy
from ..ll_api import sps, sr
from .hl_api_types import CollocatedSynapses, NodeCollection
from .hl_api_connection_helpers import _process_syn_spec, _process_spatial_projections, _connect_layers_needed
from ..synapsemodels.hl_api_synapsemodels import SynapseModel

__all__ = [
    'AllToAll',
    'ArrayConnect',
    'BuildNetwork',
    'Connect',
    'Conngen',
    'FixedIndegree',
    'FixedOutdegree',
    'FixedTotalNumber',
    'OneToOne',
    'PairwiseBernoulli',
    'reset_projection_collection',
    'SymmetricPairwiseBernoulli',
]


class Projection(object):
    """
    Projection base class representing individual projections between NodeCollections.

    Used by :py:func:`Connect` and :py:func:`BuildNetwork`. The projection will
    be put in a buffer when called with :py:func:`Connect`. The actual connections
    will be made when :py:func:`BuildNetwork` is called.

    Parameters
    ----------
    source: NodeCollection
        presynaptic nodes
    target: NodeCollection
        postsynaptic nodes
    allow_autapses: bool
        are autapses allowed
    allow_multapses: bool
        are multapses allowed
    syn_spec: :py:class:`.SynapseModel<nest.synapsemodels.hl_api_synapsemodels.SynapseModel>`
        class representating the synapse model
    kwargs: (optional)
        keyword arguments representing further connection specifications
    """

    conn_spec = {}  # Filled by subclass

    def __init__(self, source, target, allow_autapses, allow_multapses, syn_spec, **kwargs):
        self.source = source
        self.target = target
        self.syn_spec = syn_spec
        self.conn_spec.update(kwargs)

        # Parse allow_autapses and allow_multapses
        for param, name in ((allow_autapses, 'allow_autapses'),
                            (allow_multapses, 'allow_multapses')):
            if param is not None and type(param) is bool:
                self.conn_spec[name] = param

        self.use_connect_arrays = False

    def __getattr__(self, attr):
        if attr in ['source', 'target', 'conn_spec', 'syn_spec', 'use_connect_arrays']:
            return super().__getattribute__(attr)
        if attr in self.conn_spec:
            return self.conn_spec[attr]
        else:
            raise AttributeError(f'{attr} is not a connection- or synapse-specification')

    def __setattr__(self, attr, value):
        if attr in ['source', 'target', 'conn_spec', 'syn_spec', 'use_connect_arrays']:
            return super().__setattr__(attr, value)
        else:
            self.conn_spec[attr] = value

    def __str__(self):
        output = f'source: {self.source} \ntarget: {self.target} \nconn_spec: {self.conn_spec} \nsyn_spec: {self.syn_spec}'  # noqa
        return output

    def clone(self):
        """Clone object."""
        return copy.copy(self)

    def to_list(self):
        """Convert object to list.

        Projection connection expects syn_spec to be a list of dicts. Because of the different forms syn_spec
        can come in, this requires some processing.

        TODO: Can this be cleaned up?"""

        syn_spec = self.syn_spec.to_dict() if issubclass(type(self.syn_spec), SynapseModel) else self.syn_spec
        syn_spec = _process_syn_spec(syn_spec, self.conn_spec, len(self.source), len(self.target), False)

        if syn_spec is None:
            syn_spec = {'synapse_model': 'static_synapse'}
        elif isinstance(syn_spec, dict) and 'synapse_model' not in syn_spec:
            syn_spec['synapse_model'] = 'static_synapse'
        return [self.source, self.target, self.conn_spec, syn_spec]


class ProjectionCollection(object):
    """
    Class for buffering all projections.
    
    When :py:func:`Connect` is called with a :py:class:`Projection`, the projection
    is added to `_batch_projections` and stored until :py:func:`BuildNetwork` is called.
    """

    def __init__(self):
        self.reset()
        self.network_built = False

    def reset(self):
        self._batch_projections = []

    def get(self):
        return self._batch_projections

    def add(self, projection):
        self._batch_projections.append(projection)


projection_collection = ProjectionCollection()


def Connect(projection):
    """
    Connect `pre` nodes to `post` nodes.

    TODO: needs to be re-written. Parts of this can probably be moved to other functions/classes.

    Nodes in `pre` and `post` are connected using the specified connectivity
    (`all-to-all` by default) and synapse type (:cpp:class:`static_synapse <nest::static_synapse>` by default).
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
    random parameters, spatial parameters and distributions (only accesseable for nodes with spatial positions),
    logical expressions and mathematical expressions, which can be used to define node and connection parameters.

    To see all available parameters, see documentation defined in distributions, logic, math,
    random and spatial modules.

    See Also
    ---------
    :ref:`connection_management`
    """

    if issubclass(type(projection), Projection):
        projection_collection.add(projection)
    elif issubclass(type(projection), (list, tuple)):
        for proj in projection:
            projection_collection.add(proj)
    else:
        raise TypeError('"projection" must be a projection or a list of projections')

    if projection_collection.network_built:
        projection_collection.network_built = False


def BuildNetwork():
    """
    Build network.

    Send all projections in the `projection_collection` buffer to the kernel to create all
    connections in the buffer.

    The kernel expects a list of lists, where the inner lists should contain the respective
    projection's `source`, `target`, `conn_spec` dictionary, and `syn_spec` dictionary. For
    spatial networks, the inner lists should contain the projection's `source`, `target`,
    and spatial_projection dictionary containing connection and synapse specifications.

    If `source` and `target` are arrays, `connect_array` should be used. This will happen
    irregardles of the other projections in the buffer, and will be done in it's own call
    to the kernel.
    """
    from .hl_api_connections import _array_connect

    if not projection_collection.network_built:
        # Convert to list of lists
        projection_list = []
        print(f'Connecting {len(projection_collection.get())} projections...')
        for proj in projection_collection.get():
            projection = proj.to_list()
            source, target, conn_spec, syn_spec = projection
            if proj.use_connect_arrays:
                _array_connect(source, target, conn_spec, syn_spec)
            elif _connect_layers_needed(conn_spec, syn_spec):
                # Check that pre and post are layers
                if source.spatial is None:
                    raise TypeError("Presynaptic NodeCollection must have spatial information")
                if target.spatial is None:
                    raise TypeError("Postsynaptic NodeCollection must have spatial information")

                # Merge to a single projection dictionary because we have spatial projections,
                spatial_projections = _process_spatial_projections(conn_spec, syn_spec)
                projection_list.append([source, target, spatial_projections])
            else:
                # Convert syn_spec to list of dicts
                if isinstance(syn_spec, CollocatedSynapses):
                    syn_spec = syn_spec.syn_specs
                elif isinstance(syn_spec, dict):
                    syn_spec = [syn_spec]
                projection_list.append([source, target, conn_spec, syn_spec])

        # Call SLI function
        sps(projection_list)
        sr('connect_projections')

        # reset all projections
        projection_collection.reset()
        projection_collection.network_built = True


def reset_projection_collection():
    """Reset `projection_collection`"""
    projection_collection.reset()
    projection_collection.network_built = False


class AllToAll(Projection):
    """
    Class representing `all_to_all` connection rule.
    """
    def __init__(self, source, target, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'all_to_all'}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class ArrayConnect(Projection):
    """
    Class representing `connect_array`.

    NB! Will not be connected with other projections, we send this to C++ on it's own.
    """
    def __init__(self, source, target, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'one_to_one'}  # ArrayConnect uses an one-to-one scheme
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)

        # Check if we can convert to NodeCollection and use normal one-to-one connection routines
        if (not isinstance(source, NodeCollection) and not
           isinstance(target, NodeCollection) and
           len(set(source)) == len(source) and
           len(set(target)) == len(target)):
            self.source = NodeCollection(source)
            self.target = NodeCollection(target)
        else:
            self.use_connect_arrays = True


class Conngen(Projection):
    """
    Class representing the `conngen` connection rule.
    
    Mandatory associated parameter: `cg`
    """
    def __init__(self, source, target, allow_autapses=None, allow_multapses=None, syn_spec=None, cg=None, **kwargs):
        self.conn_spec = {'rule': 'conngen', 'cg': cg}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class FixedIndegree(Projection):
    """
    Class representing the `fixed_indegree` connection rule.
    
    Mandatory associated parameter: `indegree`
    """
    def __init__(self, source, target, indegree, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'fixed_indegree', 'indegree': indegree}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class FixedOutdegree(Projection):
    """
    Class representing the `fixed_outdegree` connection rule.
    
    Mandatory associated parameter: `outdegree`
    """
    def __init__(self, source, target, outdegree, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'fixed_outdegree', 'outdegree': outdegree}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class FixedTotalNumber(Projection):
    """
    Class representing the `fixed_total_number` connection rule.
    
    Mandatory associated parameter: `N`
    """
    def __init__(self, source, target, N, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'fixed_total_number', 'N': N}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class OneToOne(Projection):
    """
    Class representing the `one_to_one` connection rule.
    """
    def __init__(self, source, target, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'one_to_one'}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class PairwiseBernoulli(Projection):
    """
    Class representing the `pairwise_bernoulli` connection rule.
    
    Mandatory associated parameter: `p`
    """
    def __init__(self, source, target, p, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'pairwise_bernoulli', 'p': p}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class SymmetricPairwiseBernoulli(Projection):
    """
    Class representing the `symmetric_pairwise_bernoulli` connection rule.
    
    Mandatory associated parameter: `p`
    """
    def __init__(self, source, target, p, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'symmetric_pairwise_bernoulli', 'p': p}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)
