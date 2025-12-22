# -*- coding: utf-8 -*-
#
# test_glif_psc_double_alpha.py
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


@unittest.skipIf(not HAVE_SCIPY, "SciPy package is not available")
class GLIFPSCTestCase(unittest.TestCase):
    def setUp(self):
        """
        Clean up and initialize NEST before each test.
        """
        nest.ResetKernel()
        nest.resolution = 0.01
        nest.rng_seed = 123456

    def simulate_w_stim(self, model_params):
        """
        Runs a one second simulation with different types of input stimulation.
        Returns the time-steps, voltages, and collected spike times.
        """

        # Create glif model with double alpha synapses to simulate
        # For this model, tau_syn_fast = tau_syn_slow = 2.0, and amp_slow = 1.0,
        # so that this model behaves the same as glif_psc, when weight is halved
        # of the original, for the connections that go into receptor_type == 1.
        nrn = nest.Create("glif_psc_double_alpha", params=model_params)
        nrn.set({"tau_syn_fast": [2.0], "tau_syn_slow": [2.0], "amp_slow": [1.0]})

        # Create and connect inputs to glif model (weight is halved to account for double alpha synapse)
        espikes = nest.Create(
            "spike_generator", params={"spike_times": [10.0, 100.0, 400.0, 700.0], "spike_weights": [10.0] * 4}
        )
        ispikes = nest.Create(
            "spike_generator", params={"spike_times": [15.0, 99.0, 300.0, 800.0], "spike_weights": [-10.0] * 4}
        )

        cg = nest.Create(
            "step_current_generator",
            params={
                "amplitude_values": [
                    100.0,
                ],
                "amplitude_times": [
                    600.0,
                ],
                "start": 600.0,
                "stop": 900.0,
            },
        )
        pg = nest.Create("poisson_generator", params={"rate": 1000.0, "start": 200.0, "stop": 500.0})
        pn = nest.Create("parrot_neuron")

        nest.Connect(espikes, nrn, syn_spec={"receptor_type": 1})
        nest.Connect(ispikes, nrn, syn_spec={"receptor_type": 1})
        nest.Connect(cg, nrn, syn_spec={"weight": 3.0})
        nest.Connect(pg, pn)
        nest.Connect(pn, nrn, syn_spec={"weight": 17.5, "receptor_type": 1})  # This is also halved

        # For recording spikes and voltage traces
        sr = nest.Create("spike_recorder")
        nest.Connect(nrn, sr)

        mm = nest.Create("multimeter", params={"record_from": ["V_m"]})
        nest.Connect(mm, nrn)

        nest.Simulate(1000.0)

        times = mm.events["times"]
        V_m = mm.events["V_m"]
        spikes = sr.events["times"]

        return times, V_m, spikes

    def ks_assert_spikes(self, spikes, reference_spikes):
        """
        Runs a two-sided Kolmogorov-Smirnov statistic test on a set of spikes against a set of reference spikes.
        """
        p_value_lim = 0.9
        d_lim = 0.2
        d, p_value = scipy.stats.ks_2samp(spikes, reference_spikes)
        print(f"d={d}, p_value={p_value}")
        self.assertGreater(p_value, p_value_lim)
        self.assertLess(d, d_lim)

    def test_lif(self):
        """
        Check LIF model
        """
        lif_params = {
            "spike_dependent_threshold": False,
            "after_spike_currents": False,
            "adapting_threshold": False,
            "V_m": -78.85,
        }

        times, V_m, spikes = self.simulate_w_stim(lif_params)
        spikes_expected = [
            394.67,
            424.25,
            483.25,
            612.99,
            628.73,
            644.47,
            660.21,
            675.95,
            691.69,
            706.3,
            722.0,
            737.74,
            753.48,
            769.22,
            784.96,
            800.7,
            816.72,
            832.46,
            848.2,
            863.94,
            879.68,
            895.42,
        ]

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
            "V_m": -78.85,
        }
        times, V_m, spikes = self.simulate_w_stim(lif_r_params)
        expected_spikes = [
            394.67,
            400.67,
            424.51,
            484.48,
            613.4,
            621.31,
            629.67,
            638.47,
            647.7,
            657.36,
            667.42,
            677.87,
            688.67,
            699.8,
            709.84,
            721.58,
            733.57,
            745.76,
            758.11,
            770.6,
            783.21,
            795.92,
            811.36,
            824.05,
            836.81,
            849.64,
            862.54,
            875.49,
            888.48,
        ]

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
            "V_m": -78.85,
        }

        times, V_m, spikes = self.simulate_w_stim(lif_asc_params)
        expected_spikes = [394.67, 484.81, 614.85, 648.28, 685.18, 725.06, 769.76, 821.69, 876.09]

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
            "V_m": -78.85,
        }

        times, V_m, spikes = self.simulate_w_stim(lif_r_asc_params)
        expected_spikes = [394.67, 485.07, 615.16, 649.52, 689.1, 734.18, 786.34, 845.85]
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
            "V_m": -78.85,
        }

        times, V_m, spikes = self.simulate_w_stim(lif_r_asc_a_params)
        expected_spikes = [395.44, 615.27, 653.77, 701.42, 768.22, 859.82]

        self.ks_assert_spikes(spikes, expected_spikes)
        self.assertAlmostEqual(V_m[0], -78.85)

    def test_double_alpha_synapse(self):
        """
        Check double alpha synapse
        """
        neuron = nest.Create("glif_psc_double_alpha")

        # create 3 spikes as inputs
        input1 = nest.Create("spike_generator", 1, {"spike_times": [10.0]})
        input2 = nest.Create("spike_generator", 1, {"spike_times": [210.0]})
        input3 = nest.Create("spike_generator", 1, {"spike_times": [410.0]})

        # input1: double alpha at the same location with same amplitude. should double the amplitude
        # input2: double alpha at the same location with different amplitude. net effect shoulld 1.5 of single
        # input3: double alpha at different locations. The peak of the combined input shifts to ther right.
        neuron_settings = {
            "tau_syn_fast": [2.0, 2.0, 2.0],
            "tau_syn_slow": [2.0, 2.0, 4.0],
            "amp_slow": [1.0, 0.5, 0.5],
        }

        neuron.set(neuron_settings)
        # record the current with 0.1 ms resolution to capture the peak values.
        multimeter = nest.Create("multimeter", params={"record_from": ["I_syn"], "interval": 0.1})

        # set the delay to 1 ms. and amplitude is 1.
        # the expected peak time of the fast components are, 13 ms, 213 ms, 413 ms.
        nest.Connect(input1, neuron, syn_spec={"weight": 1.0, "delay": 1.0, "receptor_type": 1})
        nest.Connect(input2, neuron, syn_spec={"weight": 1.0, "delay": 1.0, "receptor_type": 2})
        nest.Connect(input3, neuron, syn_spec={"weight": 1.0, "delay": 1.0, "receptor_type": 3})
        nest.Connect(multimeter, neuron)

        # Do simulation.
        nest.Simulate(500.0)
        times = multimeter.events["times"]
        I_syn = multimeter.events["I_syn"]

        # Check the results.
        # the peak timing should be 13 ms (index 129), 213 ms (index 2129),
        # and slightly above 413 ms (413.4 ms, or index 4133).

        # for the first two we can estimate the exact peak values (2 and 1.5 pA)
        self.assertAlmostEqual(I_syn[129], 2.0, delta=0.0001)
        self.assertAlmostEqual(I_syn[2129], 1.5, delta=0.0001)
        # for the last one, check if the the new peak is larger than peak of the fast component
        self.assertGreater(I_syn[4133], I_syn[4129])  # peak is shifted to the right.


def suite():
    return unittest.makeSuite(GLIFPSCTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
