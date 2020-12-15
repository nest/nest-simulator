# -*- coding: utf-8 -*-
#
# test_siegert_neuron.py
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

# This script tests the siegert_neuron in NEST.

import nest
import unittest
import numpy as np

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")


@nest.ll_api.check_stack
@unittest.skipIf(not HAVE_GSL, "GSL is not available")
class SiegertNeuronTestCase(unittest.TestCase):
    """
    Test siegert_neuron

    Details
    -------
    Compares the rate of a white-noise-driven iaf_psc_delta neuron
    with the prediction from the Siegert neuron and analytical results.
    """

    def setUp(self):
        """
        Clean up and initialize NEST before each test.
        """
        # test parameter to compare analytic solution to simulation
        self.rtol = 1e-1

        # parameters of driven integrate-and-fire neurons
        self.N = 50
        lif_params = {"V_th": -55., "V_reset": -70., "E_L": -70.,
                      "tau_m": 10.0, "t_ref": 2.0, "C_m": 250.}
        self.lif_params = lif_params

        # simulation parameters
        master_seed = 123456
        self.simtime = 600.
        self.dt = 0.01
        self.start = 100.

        # reset kernel
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        N_vp = nest.GetKernelStatus(["total_num_virtual_procs"])[0]
        nest.SetKernelStatus({"resolution": self.dt, "use_wfr": False,
                              "grng_seed": master_seed,
                              "rng_seeds": range(master_seed + 1,
                                                 master_seed + N_vp + 1)})

    def simulate_fix_input_stats(self, mu, sigma):
        """
        Simulate with fixed input statistics mu and sigma.
        """
        # create and connect driven integrate-and-fire neurons
        self.iaf_psc_delta = nest.Create("iaf_psc_delta", self.N,
                                         params=self.lif_params)
        self.noise_generator = nest.Create("noise_generator",
                                           params={"dt": self.dt})
        self.spike_recorder = nest.Create("spike_recorder",
                                          params={"start": self.start})
        nest.Connect(self.noise_generator, self.iaf_psc_delta)
        nest.Connect(self.iaf_psc_delta, self.spike_recorder)

        # create and connect driven Siegert neuron
        lif_params = self.lif_params
        siegert_params = {"tau_m": lif_params["tau_m"],
                          "t_ref": lif_params["t_ref"],
                          "theta": lif_params["V_th"] - lif_params["E_L"],
                          "V_reset": lif_params["V_reset"] - lif_params["E_L"]}
        self.siegert_neuron = nest.Create("siegert_neuron", 1,
                                          params=siegert_params)
        self.siegert_drive = nest.Create("siegert_neuron", 1,
                                         params={"mean": 1.0, "rate": 1.0})
        self.multimeter = nest.Create("multimeter",
                                      params={"record_from": ["rate"],
                                              "interval": self.dt})
        syn_dict = {"drift_factor": mu, "diffusion_factor": sigma**2,
                    "synapse_model": "diffusion_connection"}
        nest.Connect(self.siegert_drive, self.siegert_neuron,
                     syn_spec=syn_dict)
        nest.Connect(self.multimeter, self.siegert_neuron)

        # set output statistics of noise generator
        # - for dt_scaling factor see doc/model_details/noise_generator.ipynb
        # - takes var(V) = sigma^2 / 2 into account
        lif_params = self.lif_params
        mV_to_pA = lif_params["C_m"] / lif_params["tau_m"]
        exp_dt = np.exp(-self.dt/lif_params["tau_m"])
        dt_scaling = np.sqrt((1 + exp_dt) / (1 - exp_dt))
        mean = mV_to_pA * mu
        std = mV_to_pA * sigma * dt_scaling / np.sqrt(2)
        nest.SetStatus(self.noise_generator, {"mean": mean, "std": std})

        # set initial membrane voltage distribution with stationary statistics
        nest.SetStatus(self.iaf_psc_delta, {"V_m":
                       nest.random.normal(mean=mu, std=sigma/np.sqrt(2))})

        # simulate
        nest.Simulate(self.simtime)

        # get rate prediction from Siegert neuron
        events = nest.GetStatus(self.multimeter)[0]["events"]
        senders = events["senders"]
        rate_mask = np.where(senders == self.siegert_neuron.get("global_id"))
        rate_prediction = events["rate"][rate_mask][-1]

        # get rate of integrate-and-fire neuron
        n_spikes = nest.GetStatus(self.spike_recorder)[0]["n_events"]
        rate_iaf = n_spikes / ((self.simtime - self.start) * 1e-3) / self.N

        return rate_prediction, rate_iaf

    def calculate_noisefree_analytics(self, mu):
        """
        Output rate for mean input mu in the noisefree limit sigma -> 0.
        """
        V_th = self.lif_params["V_th"] - self.lif_params["E_L"]
        V_reset = self.lif_params["V_reset"] - self.lif_params["E_L"]
        tau_m = self.lif_params["tau_m"]
        t_ref = self.lif_params["t_ref"]
        if mu > V_th:
            return 1e3 / (t_ref + tau_m * np.log((mu - V_reset)/(mu - V_th)))
        else:
            return 0.

    def test_RatePredictionAtThreshold(self):
        """
        Check the rate prediction of the Siegert neuron with
        mean exactly at threshold.
        """
        mu = self.lif_params["V_th"] - self.lif_params["E_L"]
        sigma = np.sqrt(0.1 * mu)

        # test rate prediction against simulation
        rate_prediction, rate_iaf = self.simulate_fix_input_stats(mu, sigma)
        self.assertTrue(np.isclose(rate_iaf, rate_prediction, rtol=self.rtol))
        # test rate prediction against hard coded result
        rate_prediction_test = 27.1095934379
        self.assertTrue(np.isclose(rate_prediction_test, rate_prediction))

    def test_RatePredictionSuprathresholdNoisefree(self):
        """
        Check the rate prediction of the Siegert neuron with
        mean above threshold close to the noise free limit.
        """
        mu = 1.5 * (self.lif_params["V_th"] - self.lif_params["E_L"])
        sigma = 1e-3 * (self.lif_params["V_th"] - self.lif_params["E_L"])

        # test rate prediction against simulation
        rate_prediction, rate_iaf = self.simulate_fix_input_stats(mu, sigma)
        self.assertTrue(np.isclose(rate_iaf, rate_prediction, rtol=self.rtol))
        # test rate prediction against analytical result
        rate_noisefree = self.calculate_noisefree_analytics(mu)
        self.assertTrue(np.isclose(rate_noisefree, rate_prediction))

    def test_RatePredictionSubthresholdNoisefree(self):
        """
        Check the rate prediction of the Siegert neuron with
        mean below threshold close to the noise free limit.
        """
        mu = 0.9 * (self.lif_params["V_th"] - self.lif_params["E_L"])
        sigma = 1e-3 * (self.lif_params["V_th"] - self.lif_params["E_L"])

        # test rate prediction against simulation
        rate_prediction, rate_iaf = self.simulate_fix_input_stats(mu, sigma)
        self.assertTrue(np.isclose(rate_iaf, rate_prediction, rtol=self.rtol))
        # test rate prediction against analytical result
        rate_noisefree = self.calculate_noisefree_analytics(mu)
        self.assertTrue(np.isclose(rate_noisefree, rate_prediction))

    def test_RatePredictionSubthresholdNoisy(self):
        """
        Check the rate prediction of the Siegert neuron with
        mean below threshold and strong noise.
        """
        mu = 2./3. * (self.lif_params["V_th"] - self.lif_params["E_L"])
        sigma = 1.0 * (self.lif_params["V_th"] - self.lif_params["E_L"])

        # test rate prediction against simulation
        rate_prediction, rate_iaf = self.simulate_fix_input_stats(mu, sigma)
        self.assertTrue(np.isclose(rate_iaf, rate_prediction, rtol=self.rtol))

    def test_RatePredictionInhibitoryNoisy(self):
        """
        Check the rate prediction of the Siegert neuron with
        mean below reset voltage and strong noise.
        """
        mu = -1./3. * (self.lif_params["V_th"] - self.lif_params["E_L"])
        sigma = 1.5 * (self.lif_params["V_th"] - self.lif_params["E_L"])

        # test rate prediction against simulation
        rate_prediction, rate_iaf = self.simulate_fix_input_stats(mu, sigma)
        self.assertTrue(np.isclose(rate_iaf, rate_prediction, rtol=self.rtol))


def suite():
    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite1 = unittest.TestLoader().loadTestsFromTestCase(
        SiegertNeuronTestCase)
    return unittest.TestSuite([suite1])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
