# -*- coding: utf-8 -*-
#
# test_glif_psc.py
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
import numpy as np
import nest


class GLIFPSCTestCase(unittest.TestCase):

    def setUp(self):
        """
        Clean up and initialize NEST before each test.
        """
        msd = 123456
        self.resol = 0.01
        nest.ResetKernel()
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        pyrngs = [np.random.RandomState(s)
                  for s in range(msd, msd + N_vp)]
        nest.SetKernelStatus({'resolution': self.resol,
                              'grng_seed': msd + N_vp,
                              'rng_seeds': range(msd + N_vp + 1,
                                                 msd + 2 * N_vp + 1)})

    def simulate_w_stim(self, model_params):
        """
        Runs a one second simulation with different types of input stimulation.
        Returns the time-steps, voltages, and collected spike times.
        """

        # Create glif model to simulate
        nrn = nest.Create("glif_psc", params=model_params)

        # Create and connect inputs to glif model
        espikes = nest.Create("spike_generator",
                              params={"spike_times": [10.0, 100.0,
                                                      400.0, 700.0],
                                      "spike_weights": [20.0]*4})
        ispikes = nest.Create("spike_generator",
                              params={"spike_times": [15.0, 99.0,
                                                      300.0, 800.0],
                                      "spike_weights": [-20.]*4})

        cg = nest.Create("step_current_generator",
                         params={"amplitude_values": [100.0, ],
                                 "amplitude_times": [600.0, ],
                                 "start": 600.0, "stop": 900.0})
        pg = nest.Create("poisson_generator",
                         params={"rate": 1000.,
                                 "start": 200., "stop": 500.})
        pn = nest.Create("parrot_neuron")

        nest.Connect(espikes, nrn, syn_spec={"receptor_type": 1})
        nest.Connect(ispikes, nrn, syn_spec={"receptor_type": 1})
        nest.Connect(cg, nrn, syn_spec={"weight": 3.0})
        nest.Connect(pg, pn)
        nest.Connect(pn, nrn, syn_spec={"weight": 35.0,
                                        "receptor_type": 1})

        # For recording spikes and voltage traces
        sr = nest.Create('spike_recorder')
        nest.Connect(nrn, sr)

        mm = nest.Create("multimeter", params={"record_from": ["V_m"]})
        nest.Connect(mm, nrn)

        nest.Simulate(1000.0)

        times = nest.GetStatus(mm, 'events')[0]['times']
        V_m = nest.GetStatus(mm, 'events')[0]['V_m']
        spikes = nest.GetStatus(sr, 'events')[0]['times']

        return times, V_m, spikes

    def test_lif(self):
        """
        Check LIF model
        """
        lif_params = {
            "spike_dependent_threshold": False,
            "after_spike_currents": False,
            "adapting_threshold": False,
            "V_m": -78.85
        }

        times, V_m, spikes = self.simulate_w_stim(lif_params)
        spikes_expected = [270.15, 347.02, 375.91, 418.80,
                           612.99, 628.73, 644.47, 660.21,
                           675.95, 691.69, 706.30, 722.00,
                           737.74, 753.48, 769.22, 784.96,
                           800.70, 816.72, 832.46, 848.20,
                           863.94, 879.68, 895.42]
        assert(np.allclose(spikes, spikes_expected, atol=1.0e-3))
        assert(np.isclose(V_m[0], -78.85))

    def test_lif_r(self):
        """
        Check LIF_R model
        """
        lif_r_params = {
            "spike_dependent_threshold": True,
            "after_spike_currents": False,
            "adapting_threshold": False,
            "V_m": -78.85
        }
        times, V_m, spikes = self.simulate_w_stim(lif_r_params)
        expected_spikes = [270.15, 347.17, 375.92, 419.84,
                           613.21, 620.95, 629.14, 637.77,
                           646.84, 656.34, 666.25, 676.55,
                           687.22, 698.23, 707.60, 719.23,
                           731.16, 743.30, 755.61, 768.07,
                           780.65, 793.34, 809.49, 822.12,
                           834.82, 847.61, 860.47, 873.38,
                           886.34, 899.34]
        assert(np.allclose(spikes, expected_spikes, atol=1.0e-3))
        assert(np.isclose(V_m[0], -78.85))

    def test_lif_asc(self):
        """
        Check LIF_ASC model
        """
        lif_asc_params = {
            "spike_dependent_threshold": False,
            "after_spike_currents": True,
            "adapting_threshold": False,
            "V_m": -78.85
        }

        times, V_m, spikes = self.simulate_w_stim(lif_asc_params)
        expected_spikes = [270.15, 347.67, 378.24, 615.03, 648.82,
                           686.12, 726.58, 771.84, 823.78, 878.94]
        assert(np.allclose(spikes, expected_spikes, atol=1.0e-3))
        assert(np.isclose(V_m[0], -78.85))

    def test_lif_r_asc(self):
        """
        Check LIF_R_ASC model
        """
        lif_r_asc_params = {
            "spike_dependent_threshold": True,
            "after_spike_currents": True,
            "adapting_threshold": False,
            "V_m": -78.85
        }

        times, V_m, spikes = self.simulate_w_stim(lif_r_asc_params)
        expected_spikes = [270.15, 347.81, 378.32, 615.22, 649.75,
                           689.61, 735.16, 787.96, 848.19]
        assert(np.allclose(spikes, expected_spikes, atol=1.0e-3))
        assert(np.isclose(V_m[0], -78.85))

    def test_lif_r_asc_a(self):
        """
        Check LIF_R_ASC_A model
        """
        lif_r_asc_a_params = {
            "spike_dependent_threshold": True,
            "after_spike_currents": True,
            "adapting_threshold": True,
            "V_m": -78.85
        }

        times, V_m, spikes = self.simulate_w_stim(lif_r_asc_a_params)
        expected_spikes = [271.40, 348.73, 379.26, 617.30, 662.40,
                           705.92, 821.03]
        assert(np.allclose(spikes, expected_spikes, atol=1.0e-3))


def suite():
    return unittest.makeSuite(GLIFPSCTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
