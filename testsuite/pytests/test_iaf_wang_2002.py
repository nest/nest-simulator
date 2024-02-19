# -*- coding: utf-8 -*-
#
# test_iaf_wang_2002.py
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
Tests synaptic dynamics and connections of the approximate model iaf_wang_2002.

Since the neuron is conductance based, it is impossible to analytically
confirm the membrane potential, but all the synaptic currents can be
computed analytically (for the simplified implementation we use).
The integration of the membrane potential is not tested here.

Also tests that an error is correctly raised when an NMDA-connection
from neuron other than iaf_wang_2002 is made.
"""


import nest
import numpy as np
import numpy.testing as nptest
import pytest
from scipy.special import gamma, gammaincc

w_ex = 40.0
w_in = 15.0
alpha = 0.5
tau_AMPA = 2.0
tau_GABA = 5.0
tau_rise_NMDA = 1.8
tau_decay_NMDA = 100.0


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


def spiketrain_response_nmda(t, spiketrain):
    """
    Response for NMDA
    """
    tr = tau_rise_NMDA / tau_decay_NMDA
    at = alpha * tau_rise_NMDA
    expn = gammaincc(1 - tr, at) * gamma(1 - tr) * at ** (tr - 1)

    k_0 = -expn * at + at**tr * gamma(1 - tr)
    k_1 = np.exp(-alpha * tau_rise_NMDA) - 1

    response = np.zeros_like(t)
    for sp in spiketrain:
        t_ = t - 1.0 - sp
        zero_arg = t_ == 0.0
        s0 = response[zero_arg]
        w = k_0 + k_1 * s0
        response += s_soln(w, t_, tau_decay_NMDA)
    response *= w_ex
    return response


def test_wang():
    # Create 2 neurons, so that the Wang dynamics are present
    nrn1 = nest.Create(
        "iaf_wang_2002",
        {"tau_AMPA": tau_AMPA, "tau_GABA": tau_GABA, "tau_decay_NMDA": tau_decay_NMDA, "tau_rise_NMDA": tau_rise_NMDA},
    )

    nrn2 = nest.Create(
        "iaf_wang_2002",
        {
            "tau_AMPA": tau_AMPA,
            "tau_GABA": tau_GABA,
            "tau_decay_NMDA": tau_decay_NMDA,
            "tau_rise_NMDA": tau_rise_NMDA,
            "t_ref": 0.0,
        },
    )

    receptor_types = nrn1.get("receptor_types")

    pg = nest.Create("poisson_generator", {"rate": 50.0})
    sr = nest.Create("spike_recorder", {"time_in_steps": True})

    mm1 = nest.Create(
        "multimeter", {"record_from": ["V_m", "s_AMPA", "s_NMDA", "s_GABA"], "interval": 0.1, "time_in_steps": True}
    )
    mm2 = nest.Create(
        "multimeter", {"record_from": ["V_m", "s_AMPA", "s_NMDA", "s_GABA"], "interval": 0.1, "time_in_steps": True}
    )

    ampa_syn_spec = {"weight": w_ex, "receptor_type": receptor_types["AMPA"]}

    gaba_syn_spec = {"weight": w_in, "receptor_type": receptor_types["GABA"]}

    nmda_syn_spec = {"weight": w_ex, "receptor_type": receptor_types["NMDA"]}

    nest.Connect(pg, nrn1, syn_spec=ampa_syn_spec)
    nest.Connect(nrn1, sr)
    nest.Connect(nrn1, nrn2, syn_spec=ampa_syn_spec)
    nest.Connect(nrn1, nrn2, syn_spec=gaba_syn_spec)
    nest.Connect(nrn1, nrn2, syn_spec=nmda_syn_spec)
    nest.Connect(mm1, nrn1)
    nest.Connect(mm2, nrn2)

    nest.Simulate(1000.0)

    spikes = sr.get("events", "times") * nest.resolution

    # compute analytical solutions
    times = mm1.get("events", "times") * nest.resolution
    ampa_soln = spiketrain_response(times, tau_AMPA, spikes, w_ex)
    nmda_soln = spiketrain_response_nmda(times, spikes)
    gaba_soln = spiketrain_response(times, tau_GABA, spikes, np.abs(w_in))

    nptest.assert_array_almost_equal(ampa_soln, mm2.events["s_AMPA"])
    nptest.assert_array_almost_equal(gaba_soln, mm2.events["s_GABA"])
    nptest.assert_array_almost_equal(nmda_soln, mm2.events["s_NMDA"])


def test_illegal_connection_error():
    """
    Test that connecting with NMDA synapses from iaf_psc_exp throws error.
    """
    nest.ResetKernel()
    nrn1 = nest.Create("iaf_psc_exp")
    nrn2 = nest.Create("iaf_wang_2002")
    receptor_types = nrn2.get("receptor_types")
    nmda_syn_spec = {"receptor_type": receptor_types["NMDA"]}
    with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
        nest.Connect(nrn1, nrn2, syn_spec=nmda_syn_spec)
