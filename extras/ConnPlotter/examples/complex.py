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

    layerList = [('IG', 'poisson_generator', [N, N], [1., 1.]),
                 ('RG_E', 'E', [N, N], [1., 1.]),
                 ('RG_I', 'I', [N, N], [1., 1.])]

    common_connspec = {'rule': 'pairwise_bernoulli'}
    common_synspec = {'synapse_model': 'static_synapse',
                      'delay': 1.0}

    connectList = [
        ('IG', 'RG_E',
         modCopy(common_connspec, {'mask': {'circular': {'radius': 0.2}},
                                   'p': 0.8}),
         modCopy(common_synspec, {'synapse_model': 'AMPA',
                                                   'weight': 5.0})),
        ('IG', 'RG_I',
         modCopy(common_connspec, {'mask': {'circular': {'radius': 0.3}},
                                   'p': 0.4}),
         modCopy(common_synspec, {'synapse_model': 'AMPA',
                                                   'weight': 2.0})),
        ('RG_E', 'RG_E',
         modCopy(common_connspec, {'mask': {'rectangular':
                                            {'lower_left': [-0.4, -0.2],
                                             'upper_right': [0.4, 0.2]}},
                                   'p': 1.0}),
         modCopy(common_synspec, {'synapse_model': 'AMPA',
                                                   'weight': 2.0})),
        ('RG_E', 'RG_E',
         modCopy(common_connspec, {'mask': {'rectangular':
                                            {'lower_left': [-0.2, -0.4],
                                             'upper_right': [0.2, 0.4]}},
                                   'p': 1.0}),
         modCopy(common_synspec, {'synapse_model': 'NMDA',
                                                   'weight': 2.0})),
        ('RG_E', 'RG_I',
         modCopy(common_connspec, {'mask': {'circular': {'radius': 0.5}},
                                   'p': 'nest.spatial_distributions.gaussian(nest.spatial.distance, std=1.)'}),
         modCopy(common_synspec, {'synapse_model': 'AMPA',
                                                   'weight': 1.0})),
        ('RG_I', 'RG_E',
         modCopy(common_connspec, {'mask': {'circular': {'radius': 0.25}},
                                   'p': 'nest.spatial_distributions.gaussian(nest.spatial.distance, std=0.5)'}),
         modCopy(common_synspec, {'synapse_model': 'GABA_A',
                                                   'weight': -3.0})),
        ('RG_I', 'RG_E',
         modCopy(common_connspec, {'mask': {'circular': {'radius': 0.5}},
                                   'p': '0.5*nest.spatial_distributions.gaussian(nest.spatial.distance, std=0.3)'}),
         modCopy(common_synspec, {'synapse_model': 'GABA_B',
                                                   'weight': -1.0})),
        ('RG_I', 'RG_I',
         modCopy(common_connspec, {'mask': {'circular': {'radius': 1.0}},
                                   'p': 0.1}),
         modCopy(common_synspec, {'synapse_model': 'GABA_A',
                                                   'weight': -0.5}))
    ]

    return layerList, connectList, modelList
