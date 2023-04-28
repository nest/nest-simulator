# -*- coding: utf-8 -*-
#
# test_aeif_cond_beta_multisynapse.py
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

"""
This test creates a multisynapse neuron with three receptor ports with
different synaptic rise times and decay times, and connect it to
two excitatory and one inhibitory signals. At the end, the script compares
the simulated values of V(t) with an approximate analytical formula, which
can be derived as follows:

For small excitatory inputs the synaptic current can be approximated as

I(t)=g(t)[Vrest-Eex]

where g(t) is the synaptic conductance, Vrest is the resting potential and
Eex is the excitatory reverse potential (see Roth and van Rossum, p. 144).

Using the LIF model, the differential equation for the membrane potential
can be written as

tau_m dv/dt = -v + G

where tau_m = Cm/gL, v = Vm - Vrest, and G=g(t)(Eex-Vrest)/gL
Using a first-order Taylor expansion of v around a generic time t0:

v(t0+tau_m)=v(t0)+tau_m dv/dt + O(tau_m^2)

and substituting t=t0+tau_m we get

v(t)=G(t-tau_m)

This approximation is valid for small excitatory inputs if tau_m is small
compared to the time scale of variation of G(t). Basically, this happens
when the synaptic rise time and decay time are much greater than tau_m.
An analogous approximation can be derived for small inhibitory inputs.

References
----------

A. Roth and M. C. W. van Rossum, Modeling synapses, in Computational
Modeling Methods for Neuroscientists, MIT Press 2013, Chapter 6, pp. 139-159
"""

import math
import nest
import numpy as np
import pytest


@pytest.mark.skipif_missing_gsl
class TestAeifCondBetaMultisynapse:

    def test_aeif_cond_beta_multisynapse(self, have_plotting):
        simulation_t = 300.   # ms
        dt = 0.1    # time step

        spike_time = 10.

        V_peak = 0.
        a = 4.
        b = 80.5
        E_rev = [20.0, 0.0, -85.0]    # synaptic reversal potentials
        tau_decay = [40.0, 20.0, 30.0]   # synaptic decay times
        tau_rise = [20.0, 10.0, 5.0]   # synaptic rise times
        weights = [0.01, 0.02, 0.015]   # synaptic weights
        delays = [1., 100., 130.]   # ms - synaptic delays
        Vrest = -70.6    # resting potential
        g_L = 300.0

        sg = nest.Create("spike_generator", params={"spike_times": [spike_time]})
        vm = nest.Create("voltmeter", params={"interval": dt})

        # Create one aeif_conf_beta_multisynapse neuron
        multisynapse_neuron = nest.Create("aeif_cond_beta_multisynapse", params={"E_rev": E_rev,
                                                                                 "tau_decay": tau_decay,
                                                                                 "tau_rise": tau_rise,
                                                                                 "V_peak": V_peak,
                                                                                 "a": a,
                                                                                 "b": b,
                                                                                 "E_L": Vrest,
                                                                                 "g_L": g_L})

        # create an array of synaptic indexes
        synapses_idx = range(len(weights))

        # connect spike generator to each port
        for i in synapses_idx:
            nest.Connect(sg, multisynapse_neuron, syn_spec={"delay": delays[i],
                                                            "weight": weights[i],
                                                            "receptor_type": i + 1})

        # connect voltmeter
        nest.Connect(vm, multisynapse_neuron)

        # run simulation
        nest.Simulate(simulation_t)

        # Get the membrane voltage of the multisynapse neuron
        ts = vm.events["times"]
        Vms = vm.events["V_m"]

        if have_plotting:
            # plot timeseries as a sanity check
            import matplotlib.pyplot as plt

            fig, ax = plt.subplots(nrows=6)
            ax[0].plot(ts,
                       Vms,
                       label="V_m multisyn",
                       alpha=.5)

        V_m_summed = 0.

        # loop over synapse index
        for i in range(3):
            t0 = spike_time + delays[i]    # peak starting time

            # Evaluates the driving force
            Vforce = E_rev[i] - Vrest

            # coefficient for approximate linear relationship between V(t) and g(t)
            coeff = weights[i] * Vforce / g_L

            # peak time
            t_p = tau_decay[i] * tau_rise[i] / (tau_decay[i] - tau_rise[i]) * np.log(tau_decay[i] / tau_rise[i])

            # normalization coefficient for synaptic conductance
            g0 = coeff / (np.exp(-t_p / tau_decay[i]) - np.exp(-t_p / tau_rise[i]))

            # approximate analytical calculation of V(t)
            Vtheor = g0 * (np.exp(-(ts - t0) / tau_decay[i]) - np.exp(-(ts - t0) / tau_rise[i]))
            Vtheor[ts < t0] = 0.
            V_m_summed += Vtheor
            Vtheor += Vrest

            if have_plotting:
                ax[i + 1].plot(ts, Vtheor)

                # for i in range(4):
                #     ax[i+1].plot(singlesynapse_neuron_vm[i].get("events")["times"],
                #                  singlesynapse_neuron_vm[i].get("events")["V_m"],
                #                  label="V_m single (" + str(i) + ")")

                for _ax in ax:
                    _ax.legend()

        V_m_summed += Vrest

        if have_plotting:
            ax[0].plot(ts, V_m_summed, label="summed")

            ax[-1].semilogy(ts, np.abs(Vms - V_m_summed), label="error")
            fig.savefig("/tmp/test_aeif_cond_beta_multisynapse.png")

        # large testing tolerance due to approximation (see documentation of the test)
        np.testing.assert_allclose(Vms, V_m_summed, atol=0., rtol=1E-5)
        np.testing.assert_allclose(Vms, V_m_summed, atol=1E-3)

    @pytest.mark.parametrize("t_ref", [0., .1])
    def test_refractoriness_clamping(self, t_ref):
        """test for t_ref == 0"""
        nest.ResetKernel()
        nest.resolution = .1
        V_reset = -111.

        nrn = nest.Create("aeif_cond_beta_multisynapse", params={"w": 0.,
                                                                 "a": 0.,
                                                                 "b": 0.,
                                                                 "Delta_T": 0.,
                                                                 "t_ref": t_ref,
                                                                 "I_e": 1000.,
                                                                 "V_reset": V_reset})

        sr = nest.Create("spike_recorder", params={"time_in_steps": True})
        vm = nest.Create("voltmeter", params={"time_in_steps": True,
                                              "interval": nest.resolution})

        nest.Connect(nrn, sr)
        nest.Connect(vm, nrn)

        nest.Simulate(10.)

        stime = sr.events["times"][0] - 1    # minus one because of 1-based indexing

        # test that V_m == V_reset at spike time
        np.testing.assert_almost_equal(vm.events["V_m"][stime], V_reset)

        if t_ref == 0:
            # test that V_m > V_reset one step after spike
            assert vm.events["V_m"][stime + 1] > V_reset

        else:
            # test that V_m == V_reset one step after spike
            assert vm.events["V_m"][stime + 1] == V_reset

            # test that V_m > V_reset two steps after spike
            assert vm.events["V_m"][stime + 2] > V_reset

    def test_w_dynamics_during_refractoriness(self):
        """Test that w-dynamics during refractoriness is based on V==V_reset"""
        nest.ResetKernel()
        nest.resolution = 1.
        V_reset = -111.
        E_L = -70.
        t_ref = 100.
        a = 10.
        b = 100.
        tau_w = 1.0

        nrn = nest.Create("aeif_cond_beta_multisynapse", params={"w": 0.,
                                                                 "a": a,
                                                                 "b": b,
                                                                 "tau_w": tau_w,
                                                                 "Delta_T": 0.,
                                                                 "t_ref": t_ref,
                                                                 "I_e": 1000.,
                                                                 "E_L": E_L,
                                                                 "V_reset": V_reset})

        sr = nest.Create("spike_recorder", params={"time_in_steps": True})
        vm = nest.Create("multimeter", params={"time_in_steps": True,
                                               "interval": nest.resolution,
                                               "record_from": ["V_m", "w"]})
        nest.Connect(nrn, sr)
        nest.Connect(vm, nrn)

        nest.Simulate(50.)

        stime = sr.events["times"][0] - 1    # minus one because of 1-based indexing

        # time, voltage, w at spike
        w0 = vm.events["w"][stime]
        V0 = vm.events["V_m"][stime]
        t0 = vm.events["times"][stime] * nest.resolution

        # time, voltage, w 20 steps after spike
        w1 = vm.events["w"][stime + 20]
        V1 = vm.events["V_m"][stime + 20]
        t1 = vm.events["times"][stime + 20] * nest.resolution

        dt = t1 - t0

        assert dt < t_ref   # check that still refractory
        assert V0 == V1   # V_m clamped
        assert V0 == V_reset    # V_m clamped

        # expected w
        w_theory = w0 * np.exp(-dt / tau_w) + a * (V_reset - E_L) * (1 - np.exp(-dt / tau_w))

        np.testing.assert_allclose(w1, w_theory)

    def test_recordables(self):
        nest.ResetKernel()

        nrn = nest.Create("aeif_cond_beta_multisynapse")

        mm = nest.Create("multimeter", params={"time_in_steps": True,
                                               "interval": 1.0,
                                               "record_from": ["V_m", "w", "g_1"]})

        nest.Connect(mm, nrn)

        assert len(nrn.recordables) == 3

    def test_resize_recordables(self):
        """Test that the recordable g's change when changing the number of receptor ports"""
        nest.ResetKernel()

        E_rev1 = [0.0, 0.0, -85.0]
        E_rev2 = [0.0, 0.0]
        E_rev3 = [0.0, 0.0, -85.0, 0.]
        tau_rise1 = [5.0, 1.0, 25.0]
        tau_rise2 = [5.0, 1.0]
        tau_rise3 = [5.0, 1.0, 25.0, 50.]
        tau_decay1 = [20.0, 10.0, 85.0]
        tau_decay2 = [20.0, 10.0]
        tau_decay3 = [20.0, 10.0, 85.0, 100.]

        nrn = nest.Create("aeif_cond_beta_multisynapse", params={"E_rev": E_rev1,
                                                                 "tau_rise": tau_rise1,
                                                                 "tau_decay": tau_decay1})
        assert len(nrn.recordables) == 5

        nrn.set({"E_rev": E_rev2,
                 "tau_rise": tau_rise2,
                 "tau_decay": tau_decay2})
        assert len(nrn.recordables) == 4

        nrn.set({"E_rev": E_rev3,
                 "tau_rise": tau_rise3,
                 "tau_decay": tau_decay3})
        assert len(nrn.recordables) == 6

    def test_g_beta_dynamics(self, have_plotting):
        """
        Test that g has beta function dynamics when tau_rise and tau_decay are
        different, and has alpha function dynamics when they are the same
        """

        dt = 0.1     # time step

        nest.ResetKernel()
        nest.resolution = dt

        E_rev = [0.0, 0.0, -85.0, 20.]    # synaptic reversal potentials
        tau_rise = [20.0, 10.0, 5.0, 25.]    # synaptic rise times
        tau_decay = [40.0, 20.0, 30.0, 25.]    # synaptic decay times
        weight = [1.0, 0.5, 2.0, 1.0]    # synaptic weights
        delays = [1.0, 3.0, 10.0, 10.]    # ms - synaptic delays
        spike_time = 10.    # time at which the single spike occurs
        total_t = 500.   # total simulation time

        def alpha_function(t, W=1., tau=1., t0=0.):
            tdiff_over_tau = (t - t0) / tau
            tdiff_over_tau[tdiff_over_tau < 0] = 0
            return W * tdiff_over_tau * np.e * np.exp(-tdiff_over_tau)

        def beta_function(t, W=1., tau_rise=1., tau_decay=2., t0=0.):
            if math.isclose(tau_rise, tau_decay):
                return alpha_function(t, W, tau_rise, t0)

            tpeak = tau_rise * tau_decay * np.log(tau_decay / tau_rise) / (tau_decay - tau_rise)
            den = np.exp(-tpeak / tau_decay) - np.exp(-tpeak / tau_rise)
            num = np.exp(-(t - t0) / tau_decay) - np.exp(-(t - t0) / tau_rise)

            num[num < 0] = 0

            return W * num / den

        # Create the multisynapse neuron
        nrn = nest.Create("aeif_cond_beta_multisynapse", params={"w": 0.,
                                                                 "a": 0.,
                                                                 "b": 0.,
                                                                 "Delta_T": 0.,
                                                                 "t_ref": 0.,
                                                                 "I_e": 0.,
                                                                 "E_rev": E_rev,
                                                                 "tau_rise": tau_rise,
                                                                 "tau_decay": tau_decay})

        # Create a spike generator that generates a single spike
        sg = nest.Create("spike_generator", params={"spike_times": [spike_time]})
        for i in range(len(weight)):
            nest.Connect(sg, nrn, syn_spec={"delay": delays[i],
                                            "weight": weight[i],
                                            "receptor_type": i + 1})

        # Create the multimeter that will record from the conductance channels
        mm = nest.Create("multimeter", params={"interval": dt,
                                               "record_from": ["g_1", "g_2", "g_3", "g_4"]})
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
            sim_g = sim_gs[i]

            theo_g = beta_function(t, weight[i], tau_rise[i], tau_decay[i], t0)

            if have_plotting:
                # plot timeseries as a sanity check
                import matplotlib.pyplot as plt

                fig, ax = plt.subplots(nrows=2)
                ax[0].plot(t, sim_g, label="sim")
                ax[0].plot(t, theo_g, label="theory")

                for _ax in ax:
                    _ax.legend()

                fig.savefig("/tmp/test_aeif_cond_beta_multisynapse_psc_shape_ " + str(i) + ".png")

            np.testing.assert_allclose(sim_g, theo_g)
