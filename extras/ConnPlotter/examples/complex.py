# -*- coding: utf-8 -*-
#
# complex.py
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
More complex example model.
"""


def complex():
    """
    Build lists representing more complex network model.

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

    # We use the ht_neuron here, as it has AMPA, NMDA, GABA_A, GABA_B synapses
    modelList = [('ht_neuron', m, {}) for m in ['E', 'I']]

    # We also have to add an explicit synapse model for each of the four
    # synapse types, so that NEST will know how to connect to the different
    # synapses.
    import nest  # we need information from NEST here
    ht_rc = nest.GetDefaults('ht_neuron')['receptor_types']
    modelList += [('ht_synapse', syn, {'receptor_type': ht_rc[syn]})
                  for syn in ('AMPA', 'NMDA', 'GABA_A', 'GABA_B')]

    layerList = [('IG', {'columns': N, 'rows': N, 'extent': [1.0, 1.0],
                         'elements': 'poisson_generator'}),
                 ('RG', {'columns': N, 'rows': N, 'extent': [1.0, 1.0],
                         'elements': ['E', 'I']})]

    common = {'connection_type': 'divergent',
              'synapse_model': 'static_synapse',
              'delays': 1.0}
    connectList = [
        ('IG', 'RG',
         modCopy(common, {'targets': {'model': 'E'},
                          'mask': {'circular': {'radius': 0.2}},
                          'kernel': 0.8,
                          'synapse_model': 'AMPA',
                          'weights': 5.0})),
        ('IG', 'RG',
         modCopy(common, {'targets': {'model': 'I'},
                          'mask': {'circular': {'radius': 0.3}},
                          'kernel': 0.4,
                          'synapse_model': 'AMPA',
                          'weights': 2.0})),
        ('RG', 'RG',
         modCopy(common, {'sources': {'model': 'E'},
                          'targets': {'model': 'E'},
                          'mask': {'rectangular':
                                   {'lower_left': [-0.4, -0.2],
                                    'upper_right': [0.4, 0.2]}},
                          'kernel': 1.0,
                          'synapse_model': 'AMPA',
                          'weights': 2.0})),
        ('RG', 'RG',
         modCopy(common, {'sources': {'model': 'E'},
                          'targets': {'model': 'E'},
                          'mask': {'rectangular':
                                   {'lower_left': [-0.2, -0.4],
                                    'upper_right': [0.2, 0.4]}},
                          'kernel': 1.0,
                          'synapse_model': 'NMDA',
                          'weights': 2.0})),
        ('RG', 'RG',
         modCopy(common, {'sources': {'model': 'E'},
                          'targets': {'model': 'I'},
                          'mask': {'circular': {'radius': 0.5}},
                          'kernel': {'gaussian':
                                     {'p_center': 1.0,
                                      'sigma': 1.0}},
                          'synapse_model': 'AMPA',
                          'weights': 1.0})),
        ('RG', 'RG',
         modCopy(common, {'sources': {'model': 'I'},
                          'targets': {'model': 'E'},
                          'mask': {'circular': {'radius': 0.25}},
                          'kernel': {'gaussian':
                                     {'p_center': 1.0,
                                      'sigma': 0.5}},
                          'synapse_model': 'GABA_A',
                          'weights': -3.0})),
        ('RG', 'RG',
         modCopy(common, {'sources': {'model': 'I'},
                          'targets': {'model': 'E'},
                          'mask': {'circular': {'radius': 0.5}},
                          'kernel': {'gaussian':
                                     {'p_center': 0.5,
                                      'sigma': 0.3}},
                          'synapse_model': 'GABA_B',
                          'weights': -1.0})),
        ('RG', 'RG',
         modCopy(common, {'sources': {'model': 'I'},
                          'targets': {'model': 'I'},
                          'mask': {'circular': {'radius': 1.0}},
                          'kernel': 0.1,
                          'synapse_model': 'GABA_A',
                          'weights': -0.5}))
    ]

    return layerList, connectList, modelList
