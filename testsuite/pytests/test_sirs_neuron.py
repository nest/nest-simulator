# -*- coding: utf-8 -*-
#
# test_sirs_neuron.py
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

import nest
import numpy as np


class TestSirNeuron:
    r"""Test of sirs_neuron model."""

    def test_single_neuron_dynamics(self):
        r"""Test whether neuron can be infected and sustains that infection if decay is set to zero."""

        # hard code to avoid looping dynamics
        tau_m = 1
        simtime = 5

        nest.ResetKernel()

        print("Testing single neuron dynamics of sirs_neuron model")

        h_expected = [1.0] * int((simtime - 1) * 10)
        # expected output S: 1 time step in initial 0 state, then it takes tau_m to update
        # to the infected state which happens with prob. 1, then it takes tau_m to updae
        # to recovered state which happens with prob 1.
        S_expected = (
            [0.0] * int(1 + tau_m * 10)
            + [1.0] * int(tau_m * 10)
            + [2.0] * int(tau_m * 10)
            + [0.0] * int((simtime - 1) * 10 - 1 - 3 * 10 * tau_m)
        )
        # expected spike output: one spike one time step after neuron gets infected,
        # two spikes one time step after neuron leaves infectious state
        spikes_expected = [0.1 + tau_m + 0.1, 0.1 + 2 * tau_m + 0.1, 0.1 + 2 * tau_m + 0.1]

        nrn = nest.Create("sirs_neuron")
        nrn.h = 1
        nrn.mu_sirs = 1
        nrn.beta_sirs = 1
        nrn.eta_sirs = 1
        nrn.tau_m = 1

        multi = nest.Create("multimeter", {"record_from": ["S", "h"], "interval": 0.1})
        spike_recorder = nest.Create("spike_recorder")

        nest.Connect(multi, nrn)
        nest.Connect(nrn, spike_recorder)

        nest.Simulate(simtime)

        S_recorded = multi.events["S"]
        h_recorded = multi.events["h"]
        # since we only record the first (simtime - 1)ms for S and h,
        # also only record the first (simtime - 1)ms of spikes
        spikes_recorded = spike_recorder.events["times"][spike_recorder.events["times"] <= (simtime - 1)]

        assert np.allclose(h_expected, h_recorded)
        assert np.allclose(S_expected, S_recorded)
        assert np.allclose(spikes_expected, spikes_recorded)

    def test_propagation_of_infection(self, tau_m=1, simtime=5):
        r"""Test whether neuron can transmit infection."""

        nest.ResetKernel()

        print("Testing propagation of infection in sirs_neuron model")

        S_expected = (
            [0.0] * int(1 + tau_m * 10) + [0.0] * int(tau_m * 10) + [1.0] * int((simtime - 1) * 10 - 1 - 2 * 10 * tau_m)
        )
        # expected output S: 1 time step in initial 0 state, then it takes tau_m to update
        # neuron 1 to the infected state during which neuron 2 stays in the susceptible 0 state,
        # then it takes tau_m to update neuron 2 to the infected state

        nrn_1 = nest.Create("sirs_neuron")
        nrn_2 = nest.Create("sirs_neuron")

        # infect nrn_1 at time 0.1ms + tau_m
        nrn_1.h = 1
        nrn_1.beta_sirs = 1
        nrn_1.tau_m = tau_m
        nrn_1.mu_sirs = 0

        nrn_2.beta_sirs = 1
        nrn_2.tau_m = tau_m
        nrn_2.mu_sirs = 0

        nest.Connect(nrn_1, nrn_2)

        multi = nest.Create("multimeter", {"record_from": ["S"], "interval": 0.1})
        nest.Connect(multi, nrn_2)
        nest.Simulate(simtime)
        S_recorded = multi.events["S"]
        assert np.allclose(S_expected, S_recorded)


if __name__ == "__main__":
    test = TestSirNeuron()
    test.test_single_neuron_dynamics()
    test.test_propagation_of_infection()
