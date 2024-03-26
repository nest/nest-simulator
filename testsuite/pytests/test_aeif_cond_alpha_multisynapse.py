# -*- coding: utf-8 -*-
#
# test_aeif_cond_alpha_multisynapse.py
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
import pytest


@pytest.mark.skipif_missing_gsl
class TestAeifCondAlphaMultisynapse:
    r"""
    This test creates a multisynapse neuron and first checks if time constants
    can be set correctly.

    Afterwards, it simulates the mutisynapse neuron with n (n=4) different time
    constants and records the neuron's synaptic current. At the same time, it simulates
    n (n=4) single synapse neurons with according parameters.
    At the end, it compares the multisynapse neuron currents with each according single
    synapse current.
    """

    def test_single_multi_synapse_equivalence(self, have_plotting, report_dir):
        simulation_t = 2500.0  # total simulation time [ms]

        dt = 0.1  # time step [ms]

        E_ex = 0.0  # excitatory reversal potential [mV]
        E_in = -85.0  # inhibitory reversal potential [mV]
        V_peak = 0.0  # spike detection threshold [mV]
        a = 4.0
        b = 80.5
        tau_syn = [0.2, 0.5, 1.0, 10.0]  # synaptic times [ms]
        weight = [1.0, 5.0, 1.0, -1.0]  # synaptic weights
        E_rev = [E_ex, E_ex, E_ex, E_in]  # synaptic reversal potentials [mV]
        spike_time = 1.0  # time at which the single spike occurs [ms]

        # The delays have to be ordered and needs enough space between them to avoid one PSC from affecting the next
        delays = [1.0, 500.0, 1500.0, 2250.0]  # ms

        V_m_steadystate = -70.59992755
        w_steadystate = 0.00029113

        nest.ResetKernel()
        nest.resolution = dt

        sg = nest.Create("spike_generator", params={"spike_times": [spike_time]})

        # Create the several aeif_conf_alpha (one for each synapse of the multisynapse neuron)
        singlesynapse_neuron = len(weight) * [None]
        singlesynapse_neuron_vm = len(weight) * [None]
        for i in range(len(weight)):
            node = nest.Create(
                "aeif_cond_alpha",
                params={
                    "V_m": V_m_steadystate,
                    "w": w_steadystate,
                    "V_peak": V_peak,
                    "a": a,
                    "b": b,
                    "tau_syn_ex": tau_syn[i],
                    "tau_syn_in": tau_syn[i],
                },
            )
            singlesynapse_neuron[i] = node

            nest.Connect(sg, node, syn_spec={"delay": delays[i], "weight": weight[i]})

            vm = nest.Create("voltmeter", params={"interval": dt})
            singlesynapse_neuron_vm[i] = vm

            nest.Connect(vm, node)

        # Create one aeif_conf_alpha_multisynapse
        multisynapse_neuron = nest.Create(
            "aeif_cond_alpha_multisynapse",
            params={
                "V_m": V_m_steadystate,
                "w": w_steadystate,
                "tau_syn": tau_syn,
                "E_rev": E_rev,
                "V_peak": V_peak,
                "a": a,
                "b": b,
            },
        )

        # connect the spike generator to the multisynapse neuron with four neurons with different delays
        for i in range(len(weight)):
            nest.Connect(
                sg, multisynapse_neuron, syn_spec={"delay": delays[i], "weight": abs(weight[i]), "receptor_type": i + 1}
            )

        # Create one voltmeter for the multisynapse neuron
        multisynapse_neuron_vm = nest.Create("voltmeter", params={"interval": dt})

        nest.Connect(multisynapse_neuron_vm, multisynapse_neuron)

        nest.Simulate(simulation_t)

        summed_V_m = np.zeros_like(multisynapse_neuron_vm.get("events")["V_m"], dtype=float)
        for i in range(4):
            summed_V_m += singlesynapse_neuron_vm[i].get("events")["V_m"] - V_m_steadystate

        summed_V_m += V_m_steadystate

        error = np.abs(summed_V_m - multisynapse_neuron_vm.get("events")["V_m"])

        if have_plotting:
            # plot timeseries as a sanity check
            import matplotlib.pyplot as plt

            fig, ax = plt.subplots(nrows=6)
            ax[0].plot(
                multisynapse_neuron_vm.get("events")["times"],
                multisynapse_neuron_vm.get("events")["V_m"],
                label="V_m multisyn",
                alpha=0.5,
            )
            ax[0].plot(multisynapse_neuron_vm.get("events")["times"], summed_V_m, label="V_m summed", alpha=0.5)

            for i in range(4):
                ax[i + 1].plot(
                    singlesynapse_neuron_vm[i].get("events")["times"],
                    singlesynapse_neuron_vm[i].get("events")["V_m"],
                    label="V_m single (" + str(i) + ")",
                )

            for _ax in ax:
                _ax.legend()

            ax[-1].semilogy(multisynapse_neuron_vm.get("events")["times"], error, label="errror")

            fig.savefig(report_dir / "test_aeif_cond_alpha_multisynapse.png")

        # compare with a large tolerance because previous PSPs affect subsequent PSPs in the multisynapse neuron
        np.testing.assert_allclose(error, 0, atol=1e-6)

    def test_recordables(self):
        r"""Test that the right number of recordables are created when setting ``record_from``."""
        nest.ResetKernel()

        nrn = nest.Create("aeif_cond_alpha_multisynapse")

        mm = nest.Create(
            "multimeter", params={"time_in_steps": True, "interval": 1.0, "record_from": ["V_m", "w", "g_1"]}
        )

        nest.Connect(mm, nrn)

        assert len(nrn.recordables) == 3

    def test_resize_recordables(self):
        r"""Test that the recordable g's change when changing the number of receptor ports"""
        nest.ResetKernel()

        E_rev1 = [0.0, 0.0, -85.0]
        E_rev2 = [0.0, 0.0]
        E_rev3 = [0.0, 0.0, -85.0, 0.0]
        tau_syn1 = [5.0, 1.0, 25.0]
        tau_syn2 = [5.0, 1.0]
        tau_syn3 = [5.0, 1.0, 25.0, 50.0]

        nrn = nest.Create("aeif_cond_alpha_multisynapse", params={"E_rev": E_rev1, "tau_syn": tau_syn1})
        assert len(nrn.recordables) == 5

        nrn.set({"E_rev": E_rev2, "tau_syn": tau_syn2})
        assert len(nrn.recordables) == 4

        nrn.set({"E_rev": E_rev3, "tau_syn": tau_syn3})
        assert len(nrn.recordables) == 6

    def test_g_alpha_dynamics(self, have_plotting, report_dir):
        r"""Test that g has alpha function dynamics"""

        dt = 0.1  # time step

        nest.ResetKernel()
        nest.resolution = dt

        E_rev = [0.0, 0.0, -85.0, 20.0]  # synaptic reversal potentials
        tau_syn = [40.0, 20.0, 30.0, 25.0]  # synaptic time constants
        weight = [1.0, 0.5, 2.0, 1.0]  # synaptic weights
        delays = [1.0, 3.0, 10.0, 10.0]  # synaptic delays [ms]
        spike_time = 10.0  # time at which the single spike occurs
        total_t = 500.0  # total simulation time

        def alpha_function(t, W=1.0, tau=1.0, t0=0.0):
            tdiff_over_tau = (t - t0) / tau
            tdiff_over_tau[tdiff_over_tau < 0] = 0
            return W * tdiff_over_tau * np.e * np.exp(-tdiff_over_tau)

        # Create the multisynapse neuron
        nrn = nest.Create(
            "aeif_cond_alpha_multisynapse",
            params={
                "w": 0.0,
                "a": 0.0,
                "b": 0.0,
                "Delta_T": 0.0,
                "t_ref": 0.0,
                "I_e": 0.0,
                "E_rev": E_rev,
                "tau_syn": tau_syn,
            },
        )

        # Create a spike generator that generates a single spike
        sg = nest.Create("spike_generator", params={"spike_times": [spike_time]})
        for i in range(len(weight)):
            nest.Connect(sg, nrn, syn_spec={"delay": delays[i], "weight": weight[i], "receptor_type": i + 1})

        # Create the multimeter that will record from the conductance channels
        mm = nest.Create("multimeter", params={"interval": dt, "record_from": ["g_1", "g_2", "g_3", "g_4"]})
        nest.Connect(mm, nrn)

        nest.Simulate(total_t)

        # Get the conductances measured during the simulation
        t = mm.events["times"]
        sim_g_1 = mm.events["g_1"]
        sim_g_2 = mm.events["g_2"]
        sim_g_3 = mm.events["g_3"]
        sim_g_4 = mm.events["g_4"]
        sim_gs = [sim_g_1, sim_g_2, sim_g_3, sim_g_4]

        for i in range(4):
            t0 = spike_time + delays[i]
            W = weight[i]
            tau = tau_syn[i]
            sim_g = sim_gs[i]

            theo_g = alpha_function(t, W, tau, t0)

            if have_plotting:
                # plot timeseries as a sanity check
                import matplotlib.pyplot as plt

                fig, ax = plt.subplots(nrows=2)
                ax[0].plot(t, sim_g, label="sim")
                ax[0].plot(t, theo_g, label="theory")

                for _ax in ax:
                    _ax.legend()

                fig.savefig(report_dir / f"test_aeif_cond_alpha_multisynapse_psc_shape_{i}.png")

            np.testing.assert_allclose(sim_g, theo_g)
