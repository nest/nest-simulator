# -*- coding: utf-8 -*-
#
# test_get_sp_status.py
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
Structural Plasticity GetStatus Test
-----------------------
This tests the functionality of the GetStructuralPlasticityStatus
function
"""

import nest
import unittest

__author__ = 'sdiaz'


class TestGetStructuralPlasticityStatus(unittest.TestCase):
    neuron_model = 'iaf_psc_alpha'
    nest.CopyModel('static_synapse', 'synapse_ex')
    nest.SetDefaults('synapse_ex', {'weight': 1.0, 'delay': 1.0})
    nest.SetStructuralPlasticityStatus({
        'structural_plasticity_synapses': {
            'synapse_ex': {
                'synapse_model': 'synapse_ex',
                'post_synaptic_element': 'Den_ex',
                'pre_synaptic_element': 'Axon_ex',
            },
        }
    })

    growth_curve = {
        'growth_curve': "gaussian",
        'growth_rate': 0.0001,  # (elements/ms)
        'continuous': False,
        'eta': 0.0,  # Ca2+
        'eps': 0.05
    }

    '''
    Now we assign the growth curves to the corresponding synaptic
    elements
    '''
    synaptic_elements = {
        'Den_ex': growth_curve,
        'Den_in': growth_curve,
        'Axon_ex': growth_curve,
    }
    nodes = nest.Create(neuron_model,
                        2,
                        {'synaptic_elements': synaptic_elements}
                        )
    all = nest.GetStructuralPlasticityStatus()
    assert ('structural_plasticity_synapses' in all)
    assert ('syn1' in all['structural_plasticity_synapses'])
    assert ('structural_plasticity_update_interval' in all)
    assert (all['structural_plasticity_update_interval'] == 10000.)

    sp_synapses = nest.GetStructuralPlasticityStatus(
        'structural_plasticity_synapses'
    )
    syn = sp_synapses['syn1']
    assert ('pre_synaptic_element' in syn)
    assert ('post_synaptic_element' in syn)
    assert (syn['pre_synaptic_element'] == 'Axon_ex')
    assert (syn['post_synaptic_element'] == 'Den_ex')

    sp_interval = nest.GetStructuralPlasticityStatus(
        'structural_plasticity_update_interval'
    )
    assert (sp_interval == 10000.)


def suite():
    test_suite = unittest.makeSuite(
        TestGetStructuralPlasticityStatus,
        'test'
    )
    return test_suite


if __name__ == '__main__':
    unittest.main()
