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
from ..lib.hl_api_connections import Connect as nestlib_Connect

__all__ = [
    'Connect',
    'Connect_immediately',
    'build_network',
    'OneToOne',
    'AllToAll',
    'FixedIndegree',
    'FixedOutdegree',
    'FixedTotalNumber',
    'PairwiseBernoulli',
]


class Projection(object):
    conn_spec = {}  # Filled by subclass

    def __init__(self, source, target, allow_autapses, allow_multapses, **kwargs):
        self.source = source
        self.target = target
        self.syn_spec = kwargs

        # Parse allow_autapses and allow_multapses
        for param, name in ((allow_autapses, 'allow_autapses'),
                            (allow_multapses, 'allow_multapses')):
            if param is not None and type(param) is bool:
                self.conn_spec[name] = param

    def apply(self):
        nestlib_Connect(self.source, self.target, self.conn_spec, self.syn_spec)

    def clone(self):
        return copy.copy(self)

    def __getattr__(self, attr):
        if attr in ['source', 'target', 'conn_spec', 'syn_spec']:
            return super().__getattribute__(attr)
        if attr in self.conn_spec:
            return self.conn_spec[attr]
        elif attr in self.syn_spec:
            return self.syn_spec[attr]
        else:
            raise AttributeError(f'{attr} is not a connection- or synapse-specification')

    def __setattr__(self, attr, value):
        if attr in ['source', 'target', 'conn_spec', 'syn_spec']:
            return super().__setattr__(attr, value)
        if attr in self.conn_spec:
            self.conn_spec[attr] = value
        elif attr in self.syn_spec:
            self.syn_spec[attr] = value
        else:
            raise AttributeError(f'{attr} is not a connection- or synapse-specification')


class ProjectionCollection(object):

    def __init__(self):
        self.reset()

    def reset(self):
        self._batch_projections = []

    def get(self):
        return self._batch_projections

    def add(self, projection):
        self._batch_projections.append(projection)


projection_collection = ProjectionCollection()


def Connect(projection):
    projection_collection.add(projection)


def Connect_immediately(projections):
    projections = [projections] if issubclass(type(projections), Projection) else projections
    if not (issubclass(type(projections), (tuple, list)) and
            all([issubclass(type(x), Projection) for x in projections])):
        raise TypeError('"projections" must be a projection or a list of projections')
    for projection in projections:
        projection.apply()


def build_network():
    for projection in projection_collection.get():
        projection.apply()


class OneToOne(Projection):
    def __init__(self, source, target, allow_autapses=None, allow_multapses=None, **kwargs):
        self.conn_spec = {'rule': 'one_to_one'}
        super().__init__(source, target, allow_autapses, allow_multapses, **kwargs)


class AllToAll(Projection):
    def __init__(self, source, target, allow_autapses=None, allow_multapses=None, **kwargs):
        self.conn_spec = {'rule': 'all_to_all'}
        super().__init__(source, target, allow_autapses, allow_multapses, **kwargs)


class FixedIndegree(Projection):
    def __init__(self, source, target, indegree, allow_autapses=None, allow_multapses=None, **kwargs):
        self.conn_spec = {'rule': 'fixed_indegree', 'indegree': indegree}
        super().__init__(source, target, allow_autapses, allow_multapses, **kwargs)


class FixedOutdegree(Projection):
    def __init__(self, source, target, outdegree, allow_autapses=None, allow_multapses=None, **kwargs):
        self.conn_spec = {'rule': 'fixed_outdegree', 'outdegree': outdegree}
        super().__init__(source, target, allow_autapses, allow_multapses, **kwargs)


class FixedTotalNumber(Projection):
    def __init__(self, source, target, N, allow_autapses=None, allow_multapses=None, **kwargs):
        self.conn_spec = {'rule': 'fixed_total_number', 'N': N}
        super().__init__(source, target, allow_autapses, allow_multapses, **kwargs)


class PairwiseBernoulli(Projection):
    def __init__(self, source, target, p, allow_autapses=None, allow_multapses=None, **kwargs):
        self.conn_spec = {'rule': 'pairwise_bernoulli', 'p': p}
        super().__init__(source, target, allow_autapses, allow_multapses, **kwargs)
