# -*- coding: utf-8 -*-
#
# test_iaf_bw_2001_exact.py
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
Tests dynamics of the model iaf_bw_2001_exact.

Since the neuron is conductance based, it is impossible to analytically
confirm the membrane potential. We therefore test that without the NMDA-
currents, we get the same results as from an iaf_cond_exp neuron, and
that with NMDA-currents, we get a larger V_m.
We then test the AMPA and GABA gating variables against analytical soluton.
"""


import nest
import numpy as np
import numpy.testing as nptest
import pytest

w_ex = 40.0
w_in = 15.0


def s_soln(w, t, tau):
    """
    Solution for synaptic variables
    """
    isyn = np.zeros_like(t)
    useinds = t >= 0.0
    isyn[useinds] = w * np.exp(-t[useinds] / tau)
    return isyn


def spiketrain_response(t, tau, spiketrain, w):
    response = np.zeros_like(t)
    for sp in spiketrain:
        t_ = t - 1.0 - sp
        zero_arg = t_ == 0.0
        response += s_soln(w, t_, tau)
    return response


def test_iaf_bw_2001_exact():
    """
    Creates 4 neurons.
    bw_presyn: pre-synaptic iaf_bw_2001_exact
    bw_postsyn_1: post-synaptic iaf_bw_2001_exact, will have AMPA, GABA and NMDA synapses
    bw_postsyn_2: post-synaptic iaf_bw_2001_exact, will only have AMPA and GABA
    cond_exp_postsyn: post-synaptic iaf_cond_exp, will only have AMPA and GABA

    We test that bw_postsyn_2 and cond_exp_postsyn have identical V_m.
    We test that bw_postsyn_1 has greater V_m compared to bw_postsyn_2.
    We test that s_AMPA and s_GABA have the correct analytical solution.
    """

    cond_exp_params = {
        "tau_syn_in": 5.0,  # GABA decay time constant
        "tau_syn_ex": 2.0,  # AMPA decay time constant
        "g_L": 25.0,  # leak conductance
        "E_L": -70.0,  # leak reversal potential
        "E_ex": 0.0,  # excitatory reversal potential
        "E_in": -70.0,  # inhibitory reversal potential
        "V_reset": -55.0,  # reset potential
        "V_th": -50.0,  # threshold
        "C_m": 500.0,  # membrane capacitance
        "t_ref": 0.0,  # refreactory period
    }

    wang_params = cond_exp_params.copy()
    wang_params.pop("tau_syn_in")
    wang_params.pop("tau_syn_ex")
    wang_params.update(
        tau_GABA=cond_exp_params["tau_syn_in"],  # GABA decay time constant
        tau_AMPA=cond_exp_params["tau_syn_ex"],  # AMPA decay time constant
        tau_decay_NMDA=100.0,  # NMDA decay time constant
        tau_rise_NMDA=2.0,  # NMDA rise time constant
        alpha=0.5,  # NMDA parameter
        conc_Mg2=1.0,  # Magnesium concentration
    )

    bw_presyn = nest.Create("iaf_bw_2001_exact", wang_params)
    bw_postsyn_1 = nest.Create("iaf_bw_2001_exact", wang_params)
    bw_postsyn_2 = nest.Create("iaf_bw_2001_exact", wang_params)
    cond_exp_postsyn = nest.Create("iaf_cond_exp", cond_exp_params)

    receptor_types = bw_presyn.receptor_types

    pg = nest.Create("poisson_generator", {"rate": 50.0})
    sr = nest.Create("spike_recorder", {"time_in_steps": True})

    mm_presyn, mm_bw1, mm_bw2 = nest.Create(
        "multimeter", n=3, params={"record_from": ["V_m", "s_AMPA", "s_GABA"], "interval": 0.1, "time_in_steps": True}
    )
    mm_ce = nest.Create("multimeter", {"record_from": ["V_m"], "interval": 0.1, "time_in_steps": True})

    # for post-synaptic iaf_bw_2001_exact
    ampa_syn_spec = {"weight": w_ex, "receptor_type": receptor_types["AMPA"]}
    gaba_syn_spec = {"weight": w_in, "receptor_type": receptor_types["GABA"]}
    nmda_syn_spec = {"weight": w_ex, "receptor_type": receptor_types["NMDA"]}

    # for post-synaptic iaf_cond_exp
    ex_syn_spec = {"weight": w_ex}
    in_syn_spec = {"weight": -w_in}

    nest.Connect(pg, bw_presyn, syn_spec=ampa_syn_spec)
    nest.Connect(bw_presyn, sr)

    nest.Connect(bw_presyn, bw_postsyn_1, syn_spec=ampa_syn_spec)
    nest.Connect(bw_presyn, bw_postsyn_1, syn_spec=gaba_syn_spec)
    nest.Connect(bw_presyn, bw_postsyn_1, syn_spec=nmda_syn_spec)

    nest.Connect(bw_presyn, bw_postsyn_2, syn_spec=ampa_syn_spec)
    nest.Connect(bw_presyn, bw_postsyn_2, syn_spec=gaba_syn_spec)

    nest.Connect(bw_presyn, cond_exp_postsyn, syn_spec=ex_syn_spec)
    nest.Connect(bw_presyn, cond_exp_postsyn, syn_spec=in_syn_spec)

    nest.Connect(mm_presyn, bw_presyn)
    nest.Connect(mm_bw1, bw_postsyn_1)
    nest.Connect(mm_bw2, bw_postsyn_2)
    nest.Connect(mm_ce, cond_exp_postsyn)

    nest.Simulate(1000.0)

    spikes = sr.events["times"] * nest.resolution
    first_spike_ind = int(spikes[0] / nest.resolution)

    # compute analytical solutions
    times = mm_presyn.events["times"] * nest.resolution
    ampa_soln = spiketrain_response(times, wang_params["tau_AMPA"], spikes, w_ex)
    gaba_soln = spiketrain_response(times, wang_params["tau_GABA"], spikes, np.abs(w_in))

    nptest.assert_array_almost_equal(mm_bw2.events["V_m"], mm_ce.events["V_m"], decimal=10)
    assert (
        mm_bw1.events["V_m"][first_spike_ind + 10 :] > mm_bw2.events["V_m"][first_spike_ind + 10 :]
    ).all()  # 10 due to delay
    nptest.assert_array_almost_equal(ampa_soln, mm_bw1.events["s_AMPA"])
    nptest.assert_array_almost_equal(gaba_soln, mm_bw1.events["s_GABA"])


def test_connect_NMDA_after_simulate():
    """
    Test that error is thrown if we try to make a connection after running
    nest.Simulate() and the buffers have already been created.
    """
    presyn = nest.Create("iaf_bw_2001_exact")
    postsyn = nest.Create("iaf_bw_2001_exact")

    receptor_types = presyn.receptor_types
    nmda_syn_spec = {"weight": 1.0, "receptor_type": receptor_types["NMDA"]}

    nest.Connect(presyn, postsyn, syn_spec=nmda_syn_spec)
    nest.Simulate(1.0)
    with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
        nest.Connect(postsyn, presyn, syn_spec=nmda_syn_spec)
