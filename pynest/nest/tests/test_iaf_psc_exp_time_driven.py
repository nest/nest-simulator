# -*- coding: utf-8 -*-
#
# test_iaf_psc_exp_time_driven.py
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

import unittest
import nest
import numpy as np


class TestIafPSCExpTimeDrivenNeuron(unittest.TestCase):

    def test_valid_and_invalid_connections(self):
        nest.ResetKernel()
        n1 = nest.Create('iaf_psc_exp', 1, {'time_driven': False})
        n2 = nest.Create('iaf_psc_exp', 1, {'time_driven': False})
        nest.Connect(n1, n2, syn_spec={'model': 'static_synapse'})
        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(n1, n2, syn_spec={
                         'model': 'time_driven_static_synapse'})

        nest.ResetKernel()
        n1 = nest.Create('iaf_psc_exp', 1, {'time_driven': False})
        n2 = nest.Create('iaf_psc_exp', 1, {'time_driven': True})
        nest.Connect(n1, n2, syn_spec={'model': 'static_synapse'})
        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(n1, n2, syn_spec={
                         'model': 'time_driven_static_synapse'})

        nest.ResetKernel()
        n1 = nest.Create('iaf_psc_exp', 1, {'time_driven': True})
        n2 = nest.Create('iaf_psc_exp', 1, {'time_driven': False})
        nest.Connect(n1, n2, syn_spec={'model': 'static_synapse'})
        nest.Connect(n1, n2, syn_spec={'model': 'time_driven_static_synapse'})

        nest.ResetKernel()
        n1 = nest.Create('iaf_psc_exp', 1, {'time_driven': True})
        n2 = nest.Create('iaf_psc_exp', 1, {'time_driven': True})
        nest.Connect(n1, n2, syn_spec={'model': 'static_synapse'})
        nest.Connect(n1, n2, syn_spec={'model': 'time_driven_static_synapse'})

    def sim(self, time_driven):
        nest.ResetKernel()
        nest.SetKernelStatus({'rng_seeds': [1], 'resolution': 0.1})

        n1 = nest.Create('iaf_psc_exp', 1, {
                         'time_driven': time_driven, 'I_e': 1000.})
        n2 = nest.Create('iaf_psc_exp', 1, {
                         'time_driven': time_driven, 'I_e': 350.})

        sd = nest.Create('spike_detector')
        m = nest.Create('multimeter', 1, {'record_from': ['V_m']})

        if time_driven:
            syn_model = 'time_driven_static_synapse'
        else:
            syn_model = 'static_synapse'

        nest.Connect(n1, n2, syn_spec={'model': syn_model, 'weight': 100.})
        nest.Connect(n2, sd)
        nest.Connect(m, n2)

        nest.Simulate(500.)

        data_sd = nest.GetStatus(sd, 'events')[0]
        data_m = nest.GetStatus(m, 'events')[0]

        res = {}
        res['spike_times'] = data_sd['times']
        res['V_m'] = data_m['V_m']

        return res

    def test_compare_to_event_driven(self):

        res_event_driven = self.sim(time_driven=False)
        res_time_driven = self.sim(time_driven=True)

        self.assertTrue(np.allclose(
            res_event_driven['V_m'], res_time_driven['V_m']))
        self.assertTrue(np.allclose(
            res_event_driven['spike_times'], res_time_driven['spike_times']))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestIafPSCExpTimeDrivenNeuron)
    return suite


if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestIafPSCExpTimeDrivenNeuron)
    unittest.TextTestRunner(verbosity=2).run(suite)
