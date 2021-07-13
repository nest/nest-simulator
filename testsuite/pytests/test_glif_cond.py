# -*- coding: utf-8 -*-
#
# test_glif_cond.py
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

try:
    import scipy.stats
    HAVE_SCIPY = True
except ImportError:
    HAVE_SCIPY = False

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")


@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
@unittest.skipIf(not HAVE_SCIPY, 'SciPy package is not available')
class GLIFCONDTestCase(unittest.TestCase):

    def setUp(self):
        """
        Clean up and initialize NEST before each test.
        """
        msd = 123456
        self.resol = 0.01
        nest.ResetKernel()
        N_vp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        nest.SetKernelStatus({'resolution': self.resol,
                              'rng_seed': msd})

    def simulate_w_stim(self, model_params):
        """
        Runs a one second simulation with different types of input stimulation.
        Returns the time-steps, voltages, and collected spike times.
        """

        # Create glif model to simulate
        nrn = nest.Create("glif_cond", params=model_params)

        # Create and connect inputs to glif model
        espikes = nest.Create("spike_generator",
                              params={"spike_times": [10., 100.,
                                                      400., 700.]})
        ispikes = nest.Create("spike_generator",
                              params={"spike_times": [15., 99.,
                                                      300., 800.]})

        pg = nest.Create("poisson_generator",
                         params={"rate": 1000.,
                                 "start": 200., "stop": 500.})
        pn = nest.Create("parrot_neuron")
        cg = nest.Create("step_current_generator",
                         params={"amplitude_values": [100., ],
                                 "amplitude_times": [600., ],
                                 "start": 600., "stop": 900.})

        nest.Connect(espikes, nrn, syn_spec={"receptor_type": 1})
        nest.Connect(ispikes, nrn, syn_spec={"receptor_type": 2})
        nest.Connect(pg, pn)
        nest.Connect(pn, nrn, syn_spec={"weight": 22.,
                                        "receptor_type": 1})
        nest.Connect(pn, nrn, syn_spec={"weight": 10.,
                                        "receptor_type": 2})
        nest.Connect(cg, nrn, syn_spec={"weight": 3.})

        # For recording spikes and voltage traces
        sr = nest.Create('spike_recorder')
        nest.Connect(nrn, sr)

        mm = nest.Create("multimeter", params={"record_from": ["V_m"],
                                               "interval": self.resol})
        nest.Connect(mm, nrn)

        nest.Simulate(1000.)

        times = nest.GetStatus(mm, 'events')[0]['times']
        V_m = nest.GetStatus(mm, 'events')[0]['V_m']
        spikes = nest.GetStatus(sr, 'events')[0]['times']

        return times, V_m, spikes

    def ks_assert_spikes(self, spikes, reference_spikes):
        """
        Runs a two-sided Kolmogorov-Smirnov statistic test on a set of spikes against a set of reference spikes.
        """
        p_value_lim = 0.1
        d, p_value = scipy.stats.ks_2samp(spikes, reference_spikes)
        print(f'd={d}, p_value={p_value}')
        self.assertGreater(p_value, p_value_lim)

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
        spikes_expected = [388.04, 612.99, 628.73, 644.47, 660.21, 675.95, 691.69,
                           707.14, 722.88, 738.62, 754.36, 770.1, 785.84, 801.85,
                           817.76, 833.5, 849.24, 864.98, 880.72, 896.46]
        self.ks_assert_spikes(spikes, spikes_expected)
        self.assertAlmostEqual(V_m[0], -78.85)

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
        expected_spikes = [388.04, 613.06, 620.66, 628.7, 637.19, 646.12, 655.48, 665.26, 675.44,
                           686., 696.91, 707.68, 719.21, 731., 743.01, 755.21, 767.57, 780.07,
                           792.69, 811.13, 823.56, 836.09, 848.74, 861.48, 874.3, 887.18, 900.12]
        self.ks_assert_spikes(spikes, expected_spikes)
        self.assertAlmostEqual(V_m[0], -78.85)

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
        expected_spikes = [388.04, 613.71, 644.97, 679.45, 716.83, 758.15, 814.83, 863.82]
        self.ks_assert_spikes(spikes, expected_spikes)
        self.assertAlmostEqual(V_m[0], -78.85)

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
        expected_spikes = [388.04, 613.79, 645.19, 681.34, 722.24, 769.22, 825.66, 885.68]
        self.ks_assert_spikes(spikes, expected_spikes)
        self.assertAlmostEqual(V_m[0], -78.85)

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
        expected_spikes = [388.06, 615.24, 653.66, 701.1,  767.58, 858.97]
        self.ks_assert_spikes(spikes, expected_spikes)
        self.assertAlmostEqual(V_m[0], -78.85)


def suite():
    return unittest.makeSuite(GLIFCONDTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
