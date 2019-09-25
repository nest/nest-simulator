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

        # Create and connect inputs to
        espikes = nest.Create("spike_generator",
                              params={"spike_times": [10.0, 100.0,
                                                      400.0, 600.0],
                                      "spike_weights": [20.0]*4})
        ispikes = nest.Create("spike_generator",
                              params={"spike_times": [15.0, 99.0,
                                                      200.0, 700.0],
                                      "spike_weights": [-20.]*4})

        cg = nest.Create("step_current_generator",
                         params={"amplitude_values": [100.0, ],
                                 "amplitude_times": [500.0, ],
                                 "start": 500.0, "stop": 800.0})

        nest.Connect(espikes, nrn, syn_spec={"receptor_type": 1})
        nest.Connect(ispikes, nrn, syn_spec={"receptor_type": 1})
        nest.Connect(cg, nrn, syn_spec={"weight": 3.0})

        # For recording spikes and voltage traces
        sd = nest.Create('spike_detector')
        nest.Connect(nrn, sd)

        mm = nest.Create("multimeter", params={"record_from": ["V_m"]})
        nest.Connect(mm, nrn)

        nest.Simulate(1000.0)

        times = nest.GetStatus(mm, 'events')[0]['times']
        V_m = nest.GetStatus(mm, 'events')[0]['V_m']
        spikes = nest.GetStatus(sd, 'events')[0]['times']

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
        spikes_expected = [512.99, 528.73, 544.47, 560.21, 575.95,
                           591.69, 606.30, 622.0, 637.74, 653.48,
                           669.22, 684.96, 700.7, 716.72, 732.46,
                           748.2, 763.94, 779.68, 795.42]
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
        expected_spikes = [512.99, 520.53, 528.51, 536.94, 545.81,
                           555.12, 564.85, 574.98, 585.49, 596.35,
                           605.56, 616.96, 628.79, 640.84, 653.07,
                           665.46, 677.99, 690.63, 707.36, 719.97,
                           732.61, 745.34, 758.15, 771.03, 783.96,
                           796.94]
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
        expected_spikes = [512.99, 542.87, 575.83, 608.16, 647.63,
                           690.98, 739.12, 792.39]
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
        expected_spikes = [512.99, 542.64, 576.80, 612.05, 656.34,
                           715.03, 770.36]
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
        expected_spikes = [514.18, 549.57, 592.71, 648.15, 729.95]
        assert(np.allclose(spikes, expected_spikes, atol=1.0e-3))


def suite():
    return unittest.makeSuite(GLIFPSCTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
