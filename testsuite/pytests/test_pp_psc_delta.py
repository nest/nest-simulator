# -*- coding: utf-8 -*-
#
# test_pp_psc_delta.py
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


class PpPscDeltaTestCase(unittest.TestCase):
    """Tests for pp_psc_delta"""

    def test_rate_and_fixed_dead_time(self):
        """
        Check for reasonable firing rate and if fixed dead-time is respected
        """

        # test parameters
        d = 25.0
        lam = 10.0
        T = 10000.0

        nest.ResetKernel()
        nrn = nest.Create("pp_psc_delta")

        params = {
            "tau_m": 10.0,
            "C_m": 250.0,
            "dead_time": d,
            "dead_time_random": False,
            "dead_time_shape": 1,
            "with_reset": False,
            "tau_sfa": 34.0,
            "q_sfa": 0.0,  # // mV, reasonable default is 7 mV
            "c_1": 0.0,
            "c_2": lam,
            "c_3": 0.25,
            "I_e": 0.0,
            "t_ref_remaining": 0.0,
        }

        nrn.set(params)

        sr = nest.Create("spike_recorder")
        nest.Connect(nrn, sr)
        nest.Simulate(T)

        spikes = sr.events["times"]
        rate_sim = len(spikes) / (T * 1e-3)
        rate_ana = 1.0 / (1.0 / lam + d * 1e-3)
        ratio = rate_sim / rate_ana

        # This could fail due to bad luck. However, if it passes once,
        # then it should always do so, since the random numbers are
        # reproducible in NEST.
        self.assertLess(0.5, ratio)
        self.assertLess(ratio, 1.5)

        isi = np.diff(spikes)

        self.assertGreaterEqual(min(isi), d)

    def test_random_dead_time(self):
        """Check if random dead-time moments are respected."""

        # test parameters
        d = 50.0
        n = 10
        lam = 1.0e6
        T = 10000.0

        nest.ResetKernel()
        nrn = nest.Create("pp_psc_delta")

        params = {
            "tau_m": 10.0,
            "C_m": 250.0,
            "dead_time": d,
            "dead_time_random": True,
            "dead_time_shape": 10,
            "with_reset": False,
            "tau_sfa": 34.0,
            "q_sfa": 0.0,  # // mV, reasonable default is 7 mV
            "c_1": 0.0,
            "c_2": lam,
            "c_3": 0.25,
            "I_e": 0.0,
            "t_ref_remaining": 0.0,
        }

        nrn.set(params)

        sr = nest.Create("spike_recorder")
        nest.Connect(nrn, sr)
        nest.Simulate(T)

        spikes = sr.events["times"]
        rate_sim = len(spikes) / (T * 1e-3)
        rate_ana = 1.0 / (1.0 / lam + d * 1e-3)
        ratio = rate_sim / rate_ana

        # This could fail due to bad luck. However, if it passes once,
        # then it should always do so, since the random numbers are
        # reproducible in NEST.
        self.assertLess(0.5, ratio)
        self.assertLess(ratio, 1.5)

        isi = np.diff(spikes)

        # compute moments of ISI to get mean and variance
        isi_mean = np.mean(isi)
        isi_var = np.var(isi)

        ratio_mean = isi_mean / d
        self.assertLessEqual(0.5, ratio_mean)
        self.assertLessEqual(ratio_mean, 1.5)

        isi_var_th = n / (n / d) ** 2
        ratio_var = isi_var / isi_var_th
        self.assertLessEqual(0.5, ratio_var)
        self.assertLessEqual(ratio_var, 1.5)

    def test_adapting_threshold(self):
        """Check if threshold adaptation works by looking for negative serial
        correlation of ISI."""

        # test parameters
        d = 1e-8
        lam = 30.0
        T = 10000.0

        nest.ResetKernel()
        nrn = nest.Create("pp_psc_delta")

        params = {
            "tau_m": 10.0,
            "C_m": 250.0,
            "dead_time": d,
            "dead_time_random": False,
            "dead_time_shape": 1,
            "with_reset": False,
            "tau_sfa": 34.0,
            "q_sfa": 7.0,  # // mV, reasonable default is 7 mV
            "c_1": 0.0,
            "c_2": lam,
            "c_3": 0.25,
            "I_e": 0.0,
            "t_ref_remaining": 0.0,
        }

        nrn.set(params)

        sr = nest.Create("spike_recorder")
        nest.Connect(nrn, sr)
        nest.Simulate(T)

        spikes = sr.events["times"]

        # This could fail due to bad luck. However, if it passes once,
        # then it should always do so, since the random numbers are
        # reproducible in NEST. Adaptive threshold changes rate, thus
        # the ratio is not asserted here.
        isi = np.diff(spikes)

        isi_mean = np.mean(isi)
        isi_var = np.var(isi)
        isi_12 = np.sum(isi[:-1] * isi[1:])
        isi_corr = (isi_12 / (len(isi) - 1) - isi_mean**2) / isi_var

        self.assertLessEqual(-1.0, isi_corr)
        self.assertLessEqual(isi_corr, 0.0)

    def test_compare_adaptation_and_self_inhibition(self):
        """
        Check if threshold adaptation with membrane time constant corresponds to an
        inhibitory self-connection.
        """

        # test parameters
        d = 0.001
        lam = 10.0
        T = 200000.0
        tau_m = 25.0
        J_self = 50.0
        J_adapt = 0.1
        err = 0.2

        nest.ResetKernel()

        # create a neuron where adaptation does the reset, and one where a
        # synapse does.
        nrn1 = nest.Create("pp_psc_delta")
        nrn2 = nest.Create("pp_psc_delta")

        params1 = {
            "tau_m": tau_m,
            "C_m": 250.0,
            "dead_time": d,
            "dead_time_random": False,
            "dead_time_shape": 1,
            "with_reset": False,
            "tau_sfa": [300.0, tau_m],
            "q_sfa": [J_adapt, J_self],
            "c_1": 0.0,
            "c_2": lam,
            "c_3": 1.0,
            "I_e": 0.0,
            "t_ref_remaining": 0.0,
        }

        params2 = {
            "tau_m": tau_m,
            "C_m": 250.0,
            "dead_time": d,
            "dead_time_random": False,
            "dead_time_shape": 1,
            "with_reset": False,
            "tau_sfa": 300.0,
            "q_sfa": J_adapt,
            "c_1": 0.0,
            "c_2": lam,
            "c_3": 1.0,
            "I_e": 0.0,
            "t_ref_remaining": 0.0,
        }

        nest.SetStatus(nrn1, params1)
        nest.SetStatus(nrn2, params2)

        sr1 = nest.Create("spike_recorder")
        sr2 = nest.Create("spike_recorder")

        nest.Connect(nrn1, sr1)
        nest.Connect(nrn2, sr2)

        # Set up self-inhibitory connection for nrn2
        nest.SetDefaults("static_synapse", {"weight": -1.0 * J_self, "delay": 1.0})
        nest.Connect(nrn2, nrn2)

        nest.Simulate(T)

        n1 = nest.GetStatus(sr1)[0]["n_events"]
        n2 = nest.GetStatus(sr2)[0]["n_events"]

        ratio = float(n1) / float(n2)

        # This could fail due to bad luck. However, if it passes once,
        # then it should always do so, since the random numbers are
        # reproducible in NEST.
        self.assertLess(1.0 - err, ratio)
        self.assertLess(ratio, 1.0 + err)


def suite():
    suite = unittest.makeSuite(PpPscDeltaTestCase, "test")
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
