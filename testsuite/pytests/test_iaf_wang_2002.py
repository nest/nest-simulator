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
from scipy.special import gamma, gammaincc

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


def test_wang():
    """
    Creates 4 neurons.
    nrn1: pre-synaptic iaf_psc_wang
    nrn2: post-synaptic iaf_psc_wang, will have AMPA, GABA and NMDA synapses
    nrn3: post-synaptic iaf_psc_wang, will only have AMPA and GABA
    nrn4: post-synaptic iaf_psc_exp, will only have AMPA and GABA

    We test that nrn3 and nrn4 have identical V_m.
    We test that nrn2 has greater V_m compared to nrn3.
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

    nrn1 = nest.Create("iaf_bw_2001", wang_params)
    nrn2 = nest.Create("iaf_bw_2001", wang_params)
    nrn3 = nest.Create("iaf_bw_2001", wang_params)
    nrn4 = nest.Create("iaf_cond_exp", cond_exp_params)

    receptor_types = nrn1.get("receptor_types")

    pg = nest.Create("poisson_generator", {"rate": 50.0})
    sr = nest.Create("spike_recorder", {"time_in_steps": True})

    mm1 = nest.Create(
        "multimeter", {"record_from": ["V_m", "s_AMPA", "s_GABA"], "interval": 0.1, "time_in_steps": True}
    )
    mm2 = nest.Create(
        "multimeter", {"record_from": ["V_m", "s_AMPA", "s_GABA"], "interval": 0.1, "time_in_steps": True}
    )
    mm3 = nest.Create(
        "multimeter", {"record_from": ["V_m", "s_AMPA", "s_GABA"], "interval": 0.1, "time_in_steps": True}
    )
    mm4 = nest.Create("multimeter", {"record_from": ["V_m"], "interval": 0.1, "time_in_steps": True})

    # for post-synaptic iaf_psc_wang
    ampa_syn_spec = {"weight": w_ex, "receptor_type": receptor_types["AMPA"]}
    gaba_syn_spec = {"weight": w_in, "receptor_type": receptor_types["GABA"]}
    nmda_syn_spec = {"weight": w_ex, "receptor_type": receptor_types["NMDA"]}

    # for post-synaptic iaf_cond_exp
    ex_syn_spec = {"weight": w_ex}
    in_syn_spec = {"weight": -w_in}

    nest.Connect(pg, nrn1, syn_spec=ampa_syn_spec)
    nest.Connect(nrn1, sr)

    nest.Connect(nrn1, nrn2, syn_spec=ampa_syn_spec)
    nest.Connect(nrn1, nrn2, syn_spec=gaba_syn_spec)
    nest.Connect(nrn1, nrn2, syn_spec=nmda_syn_spec)

    nest.Connect(nrn1, nrn3, syn_spec=ampa_syn_spec)
    nest.Connect(nrn1, nrn3, syn_spec=gaba_syn_spec)

    nest.Connect(nrn1, nrn4, syn_spec=ex_syn_spec)
    nest.Connect(nrn1, nrn4, syn_spec=in_syn_spec)

    nest.Connect(mm1, nrn1)
    nest.Connect(mm2, nrn2)
    nest.Connect(mm3, nrn3)
    nest.Connect(mm4, nrn4)

    nest.Simulate(1000.0)

    spikes = sr.get("events", "times") * nest.resolution

    # compute analytical solutions
    times = mm1.get("events", "times") * nest.resolution
    ampa_soln = spiketrain_response(times, wang_params["tau_AMPA"], spikes, w_ex)
    gaba_soln = spiketrain_response(times, wang_params["tau_GABA"], spikes, np.abs(w_in))

    nptest.assert_array_equal(mm3.events["V_m"], mm4.events["V_m"])
    assert (mm2.events["V_m"] >= mm3.events["V_m"]).all()
    nptest.assert_array_almost_equal(ampa_soln, mm2.events["s_AMPA"])
    nptest.assert_array_almost_equal(gaba_soln, mm2.events["s_GABA"])


def test_illegal_connection_error():
    """
    Test that connecting with NMDA synapses from iaf_psc_exp throws error.
    """
    nest.ResetKernel()
    nrn1 = nest.Create("iaf_psc_exp")
    nrn2 = nest.Create("iaf_bw_2001")
    receptor_types = nrn2.get("receptor_types")
    nmda_syn_spec = {"receptor_type": receptor_types["NMDA"]}
    with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
        nest.Connect(nrn1, nrn2, syn_spec=nmda_syn_spec)
