# -*- coding: utf-8 -*-
#
# non_dale.py
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

# ConnPlotter --- A Tool to Generate Connectivity Pattern Matrices

"""
Non-Dale example model.

Two layer A, B, with single population each.
Both layers make excitatory and inhibitory projections
to each other, violating Dale's law.

Build with
ConnectionPattern(..., ..., synTypes=(((SynType('exc',  1.0, 'b'),
                                        SynType('inh', -1.0, 'r')),)))
"""


def non_dale():
    """
    Build lists representing non-Dale network model.

    Returns:
    layerList, connectList, modelList
    """

    def modCopy(orig, diff):
        """Create copy of dict orig, update with diff, return."""
        assert (isinstance(orig, dict))
        assert (isinstance(diff, dict))

        tmp = orig.copy()
        tmp.update(diff)
        return tmp

    N = 40

    modelList = []

    layerList = [('A', {'columns': N, 'rows': N, 'extent': [1.0, 1.0],
                        'elements': 'iaf_psc_alpha'}),
                 ('B', {'columns': N, 'rows': N, 'extent': [1.0, 1.0],
                        'elements': 'iaf_psc_alpha'})]

    common = {'connection_type': 'divergent',
              'synapse_model': 'static_synapse',
              'delays': 1.0}
    connectList = [
        ('A', 'B',
         modCopy(common, {'mask': {'circular': {'radius': 0.2}},
                          'kernel': 0.8,
                          'weights': 2.0})),
        ('A', 'B',
         modCopy(common, {'mask': {'circular': {'radius': 0.3}},
                          'kernel': 0.4,
                          'weights': -2.0})),
        ('B', 'A',
         modCopy(common, {'mask': {'rectangular':
                                   {'lower_left': [-0.4, -0.2],
                                    'upper_right': [0.4, 0.2]}},
                          'kernel': 1.0,
                          'weights': 2.0})),
        ('B', 'A',
         modCopy(common, {'mask': {'rectangular':
                                   {'lower_left': [-0.2, -0.4],
                                    'upper_right': [0.2, 0.4]}},
                          'kernel': 1.0,
                          'weights': -2.0})),
    ]

    return layerList, connectList, modelList
