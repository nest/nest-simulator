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
Connection semantics prototype functions
"""

import copy
from ..ll_api import sps, sr
from .hl_api_types import CollocatedSynapses
from .hl_api_connection_helpers import _process_syn_spec, _process_spatial_projections, _connect_layers_needed
from ..synapsemodels.hl_api_synapsemodels import SynapseModel

__all__ = [
    'AllToAll',
    'BuildNetwork',
    'Connect',
    'ConnectImmediately',
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

    def apply(self):
        # If syn_spec is a SynapseModel object it must be converted to a dictionary
        syn_spec = self.syn_spec.to_dict() if issubclass(type(self.syn_spec), SynapseModel) else self.syn_spec
        nestlib_Connect(self.source, self.target, self.conn_spec, syn_spec)

    def clone(self):
        return copy.copy(self)

    def to_list(self):
        # Projection connection expects syn_spec to be a list of dicts. Because of the different forms syn_spec
        # can come in, this requires some processing. TODO: Can this be cleaned up?
        syn_spec = self.syn_spec.to_dict() if issubclass(type(self.syn_spec), SynapseModel) else self.syn_spec
        syn_spec = _process_syn_spec(syn_spec, self.conn_spec, len(self.source), len(self.target), False)

        if syn_spec is None:
            syn_spec = {'synapse_model': 'static_synapse'}
        elif isinstance(syn_spec, dict) and 'synapse_model' not in syn_spec:
            syn_spec['synapse_model'] = 'static_synapse'
        return [self.source, self.target, self.conn_spec, syn_spec]

    def __getattr__(self, attr):
        if attr in ['source', 'target', 'conn_spec', 'syn_spec']:
            return super().__getattribute__(attr)
        if attr in self.conn_spec:
            return self.conn_spec[attr]
        else:
            raise AttributeError(f'{attr} is not a connection- or synapse-specification')

    def __setattr__(self, attr, value):
        if attr in ['source', 'target', 'conn_spec', 'syn_spec']:
            return super().__setattr__(attr, value)
        else:
            self.conn_spec[attr] = value
    
    def __str__(self):
        output = f'source: {self.source} \ntarget: {self.target} \nconn_spec: {self.conn_spec} \nsyn_spec: {self.syn_spec}'
        return output


class ProjectionCollection(object):

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
    if issubclass(type(projection), Projection):
        projection_collection.add(projection)
    elif issubclass(type(projection), (list, tuple)):
        for proj in projection:
            projection_collection.add(proj)
    else:
        raise TypeError('"projection" must be a projection or a list of projections')
    if projection_collection.network_built:
        projection_collection.network_built = False


def ConnectImmediately(projections):
    projections = [projections] if issubclass(type(projections), Projection) else projections
    if not (issubclass(type(projections), (tuple, list)) and
            all([issubclass(type(x), Projection) for x in projections])):
        raise TypeError('"projections" must be a projection or a list of projections')
    for projection in projections:
        projection.apply()


def BuildNetwork():
    if not projection_collection.network_built:
        # Convert to list of lists
        projection_list = []
        print(f'Connecting {len(projection_collection.get())} projections...')
        for projection in projection_collection.get():
            projection = projection.to_list()
            source, target, conn_spec, syn_spec = projection
            if _connect_layers_needed(conn_spec, syn_spec):
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
    projection_collection.reset()
    projection_collection.network_built = False


class AllToAll(Projection):
    def __init__(self, source, target, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'all_to_all'}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class Conngen(Projection):
    def __init__(self, source, target, allow_autapses=None, allow_multapses=None, syn_spec=None, cg=None, **kwargs):
        self.conn_spec = {'rule': 'conngen', 'cg': cg}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class FixedIndegree(Projection):
    def __init__(self, source, target, indegree, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'fixed_indegree', 'indegree': indegree}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class FixedOutdegree(Projection):
    def __init__(self, source, target, outdegree, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'fixed_outdegree', 'outdegree': outdegree}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class FixedTotalNumber(Projection):
    def __init__(self, source, target, N, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'fixed_total_number', 'N': N}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class OneToOne(Projection):
    def __init__(self, source, target, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'one_to_one'}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)


class PairwiseBernoulli(Projection):
    def __init__(self, source, target, p, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'pairwise_bernoulli', 'p': p}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)

class SymmetricPairwiseBernoulli(Projection):
    def __init__(self, source, target, p, allow_autapses=None, allow_multapses=None, syn_spec=None, **kwargs):
        self.conn_spec = {'rule': 'symmetric_pairwise_bernoulli', 'p': p}
        super().__init__(source, target, allow_autapses, allow_multapses, syn_spec, **kwargs)

