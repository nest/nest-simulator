# -*- coding: utf-8 -*-
#
# test_iaf_bw_2001.py
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
Tests dynamics and connections of the approximate model iaf_bw_2001.

Since the neuron is conductance based, it is impossible to analytically
confirm the membrane potential. We therefore test that without the NMDA-
currents, we get the same results as from an iaf_cond_exp neuron, and
that with NMDA-currents, we get a larger V_m.
We then test the AMPA and GABA gating variables against analytical solution,
and NMDA synamptic current against analytical solution.
We also test that an error is correctly raised when an NMDA-connection
from neuron other than iaf_bw_2001 is made.
"""

import nest
import numpy as np
import numpy.testing as nptest
import pytest

HAVE_BOOST = nest.ll_api.sli_func("statusdict/have_boost ::")


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
    """
    Response for AMPA/NMDA
    """

    response = np.zeros_like(t)
    for sp in spiketrain:
        t_ = t - 1.0 - sp
        zero_arg = t_ == 0.0
        response += s_soln(w, t_, tau)
    return response


@pytest.mark.skipif(not HAVE_BOOST, reason="Boost is not available")
def test_iaf_bw_2001():
    """
    Creates 4 neurons.
    nrn_presyn: pre-synaptic iaf_bw_2001
    postsyn_bw1: post-synaptic iaf_bw_2001, will have AMPA, GABA and NMDA synapses
    postsyn_bw2: post-synaptic iaf_bw_2001, will only have AMPA and GABA
    postsyn_ce: post-synaptic iaf_cond_exp, will only have AMPA and GABA

    We test that postsyn_bw2 and postsyn_ce have identical V_m.
    We test that postsyn_bw1 has greater V_m compared to postsyn_bw2.
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

    nrn_presyn = nest.Create("iaf_bw_2001", wang_params)
    postsyn_bw1 = nest.Create("iaf_bw_2001", wang_params)
    postsyn_bw2 = nest.Create("iaf_bw_2001", wang_params)
    postsyn_ce = nest.Create("iaf_cond_exp", cond_exp_params)

    receptor_types = nrn_presyn.receptor_types

    pg = nest.Create("poisson_generator", {"rate": 50.0})
    sr = nest.Create("spike_recorder", {"time_in_steps": True})

    mm_presyn, mm_bw1, mm_bw2 = nest.Create(
        "multimeter", n=3, params={"record_from": ["V_m", "s_AMPA", "s_GABA"], "interval": 0.1, "time_in_steps": True}
    )
    mm_ce = nest.Create("multimeter", {"record_from": ["V_m"], "interval": 0.1, "time_in_steps": True})

    # for post-synaptic iaf_bw_2001
    ampa_syn_spec = {"weight": w_ex, "receptor_type": receptor_types["AMPA"]}
    gaba_syn_spec = {"weight": w_in, "receptor_type": receptor_types["GABA"]}
    nmda_syn_spec = {"weight": w_ex, "receptor_type": receptor_types["NMDA"]}

    # for post-synaptic iaf_cond_exp
    ex_syn_spec = {"weight": w_ex}
    in_syn_spec = {"weight": -w_in}

    nest.Connect(pg, nrn_presyn, syn_spec=ampa_syn_spec)
    nest.Connect(nrn_presyn, sr)

    nest.Connect(nrn_presyn, postsyn_bw1, syn_spec=ampa_syn_spec)
    nest.Connect(nrn_presyn, postsyn_bw1, syn_spec=gaba_syn_spec)
    nest.Connect(nrn_presyn, postsyn_bw1, syn_spec=nmda_syn_spec)

    nest.Connect(nrn_presyn, postsyn_bw2, syn_spec=ampa_syn_spec)
    nest.Connect(nrn_presyn, postsyn_bw2, syn_spec=gaba_syn_spec)

    nest.Connect(nrn_presyn, postsyn_ce, syn_spec=ex_syn_spec)
    nest.Connect(nrn_presyn, postsyn_ce, syn_spec=in_syn_spec)

    nest.Connect(mm_presyn, nrn_presyn)
    nest.Connect(mm_bw1, postsyn_bw1)
    nest.Connect(mm_bw2, postsyn_bw2)
    nest.Connect(mm_ce, postsyn_ce)

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


@pytest.mark.skipif(not HAVE_BOOST, reason="Boost is not available")
def test_approximation_I_NMDA_V_m():
    """
    Creates 3 neurons.
    nrn_presyn: pre-synaptic iaf_bw_2001
    nrn_approx: post-synaptic iaf_bw_2001
    nrn_exact: post-synaptic iaf_bw_2001_exact

    We will check that the and membrane potentials
    of nrn_approx and nrn_exact are sufficiently close.
    """
    nest.ResetKernel()

    nrn_params = {
        "tau_GABA": 5.0,  # GABA decay time constant
        "tau_AMPA": 2.0,  # AMPA decay time constant
        "g_L": 25.0,  # leak conductance
        "E_L": -70.0,  # leak reversal potential
        "E_ex": 0.0,  # excitatory reversal potential
        "E_in": -70.0,  # inhibitory reversal potential
        "V_reset": -55.0,  # reset potential
        "V_th": -50.0,  # threshold
        "C_m": 500.0,  # membrane capacitance
        "t_ref": 0.0,  # refreactory period
    }

    nrn_presyn = nest.Create("iaf_bw_2001", nrn_params)
    nrn_approx = nest.Create("iaf_bw_2001", nrn_params)
    nrn_exact = nest.Create("iaf_bw_2001_exact", nrn_params)

    receptor_types = nrn_presyn.receptor_types

    pg = nest.Create("poisson_generator", {"rate": 150.0})
    sr = nest.Create("spike_recorder", {"time_in_steps": True})

    mm_presyn = nest.Create("multimeter", {"record_from": ["V_m", "I_NMDA"], "interval": 0.1, "time_in_steps": True})

    mm_approx = nest.Create("multimeter", {"record_from": ["V_m"], "interval": 0.1, "time_in_steps": True})
    mm_exact = nest.Create("multimeter", {"record_from": ["V_m"], "interval": 0.1, "time_in_steps": True})

    # for post-synaptic iaf_bw_2001
    ampa_syn_spec = {"weight": w_ex, "receptor_type": receptor_types["AMPA"]}
    nmda_syn_spec = {"weight": w_ex, "receptor_type": receptor_types["NMDA"]}

    nest.Connect(pg, nrn_presyn, syn_spec=ampa_syn_spec)
    nest.Connect(nrn_presyn, sr)

    nest.Connect(nrn_presyn, nrn_approx, syn_spec=nmda_syn_spec)
    nest.Connect(nrn_presyn, nrn_exact, syn_spec=nmda_syn_spec)

    nest.Connect(mm_presyn, nrn_presyn)
    nest.Connect(mm_approx, nrn_approx)
    nest.Connect(mm_exact, nrn_exact)

    nest.Simulate(500.0)

    # 0.25 was found to be roughly the tightest bound we can expect
    assert np.max(np.abs(mm_approx.events["V_m"] - mm_exact.events["V_m"])) < 0.25


@pytest.mark.skipif(not HAVE_BOOST, reason="Boost is not available")
def test_illegal_connection_error():
    """
    Test that connecting with NMDA synapses from iaf_cond_exp throws error.
    """
    nest.ResetKernel()
    nrn_ce = nest.Create("iaf_cond_exp")
    nrn_bw = nest.Create("iaf_bw_2001")
    receptor_types = nrn_bw.receptor_types
    nmda_syn_spec = {"receptor_type": receptor_types["NMDA"]}
    with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
        nest.Connect(nrn_ce, nrn_bw, syn_spec=nmda_syn_spec)
