# -*- coding: utf-8 -*-
#
# test_disconnect.py
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
__author__ = 'naveau'

import nest
import unittest


class TestDisconnectSingle(unittest.TestCase):
    def setUp(self):
        nest.ResetKernel()
        nest.set_verbosity('M_ERROR')
        self.exclude_synapse_model = ['stdp_dopamine_synapse', 'stdp_dopamine_synapse_lbl', 
                              'stdp_dopamine_synapse_hpc', 'stdp_dopamine_synapse_hpc_lbl', 
                              'gap_junction', 'gap_junction_lbl']

   
    def test_synapse_deletion_one_to_one_no_sp(self):
        for syn_model in nest.Models('synapses'):
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                nest.CopyModel('static_synapse', 'my_static_synapse')
                neurons = nest.Create('iaf_neuron', 2)
                syn_dict = {'model': syn_model}
                nest.Connect(neurons, neurons, "all_to_all", syn_dict)

                srcId = 0
                targId = 1

                conns = nest.GetConnections([neurons[srcId]],[neurons[targId]],syn_model)
                assert len(conns) == 1
                nest.DisconnectOneToOne(neurons[srcId], neurons[targId], syn_dict)

                conns = nest.GetConnections([neurons[srcId]],[neurons[targId]],syn_model)
                assert len(conns) == 0



def suite():
    test_suite = unittest.makeSuite(TestDisconnectSingle, 'test')
    return test_suite


if __name__ == '__main__':
    unittest.main()
