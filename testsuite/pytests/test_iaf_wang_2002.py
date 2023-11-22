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
Tests synaptic dynamics of iaf_wang_2002. Since the neuron is conductance based,
it is impossible to analytically confirm the membrane potential, but all the
synaptic currents can be computed analytically (for the simplified implementation
we use). The integration of the membrane potential is not tested here.
"""


import nest
import numpy as np
import numpy.testing as nptest
import pytest

def s_soln(w, t, tau):
    """
    Solution for GABA/AMPA receptors
    """
    isyn = np.zeros_like(t)
    useinds = t >= 0.
    isyn[useinds] = w * np.exp(-t[useinds] / tau)
    return isyn

def spiketrain_response(t, tau, spiketrain, w):
    response = np.zeros_like(t)
    for sp in spiketrain:
        t_ = t - 1. - sp
        zero_arg = t_ == 0.
        response += s_soln(w, t_, tau)
    return response

def spiketrain_response_nmda(t, tau, spiketrain, w, alpha):
    """
    Solution for NMDA receptors
    """
    response = np.zeros_like(t)
    for sp in spiketrain:
        t_ = t - 1. - sp
        zero_arg = t_ == 0.
        w_ = w * alpha * (1 - response[zero_arg])
        w_ = min(w_, 1 - response[zero_arg])
        response += s_soln(w_, t_, tau)
    return response

def test_wang():
    w_ex = 40.
    w_in = -15.
    alpha = 0.5
    tau_AMPA = 2.0
    tau_GABA = 5.0
    tau_NMDA = 100.0

    # Create 2 neurons, so that the Wang dynamics are present
    nrn1 = nest.Create("iaf_wang_2002", {"tau_AMPA": tau_AMPA,
                                         "tau_GABA": tau_GABA,
                                         "tau_decay_NMDA": tau_NMDA})
    
    pg = nest.Create("poisson_generator", {"rate": 50.})
    sr = nest.Create("spike_recorder", {"time_in_steps": True})
    nrn2 = nest.Create("iaf_wang_2002", {"tau_AMPA": tau_AMPA,
                                         "tau_GABA": tau_GABA,
                                         "tau_decay_NMDA": tau_NMDA,
                                         "t_ref": 0.})
    
    mm1 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "s_NMDA", "s_GABA"], "interval": 0.1})
    mm2 = nest.Create("multimeter", {"record_from": ["V_m", "s_AMPA", "s_NMDA", "s_GABA"], "interval": 0.1})
    
    ex_syn_spec = {"synapse_model": "static_synapse",
                    "weight": w_ex,
                   "receptor_type": 0}
    
    in_syn_spec = {"synapse_model": "static_synapse",
                    "weight": w_in}
    
    conn_spec = {"rule": "all_to_all"}
   
    nest.Connect(pg, nrn1, syn_spec=ex_syn_spec, conn_spec=conn_spec)
    nest.Connect(nrn1, sr)
    nest.Connect(nrn1, nrn2, syn_spec=ex_syn_spec, conn_spec=conn_spec)
    nest.Connect(nrn1, nrn2, syn_spec=in_syn_spec, conn_spec=conn_spec)
    nest.Connect(mm1, nrn1)
    
    nest.Connect(mm2, nrn2)
    
    nest.Simulate(1000.)
    
    # get spike times from membrane potential
    # cannot use spike_recorder because we abuse exact spike timing
    V_m = mm1.get("events", "V_m")
    times = mm1.get("events", "times")
    diff = np.ediff1d(V_m, to_begin=0.)
    spikes = sr.get("events", "times")
    spikes = times[diff < -3]
    
    # compute analytical solutions
    times = mm1.get("events", "times")
    ampa_soln = spiketrain_response(times, tau_AMPA, spikes, w_ex)
    nmda_soln = spiketrain_response_nmda(times, tau_NMDA, spikes, w_ex, alpha)
    gaba_soln = spiketrain_response(times, tau_GABA, spikes, np.abs(w_in))
    
    nptest.assert_array_almost_equal(ampa_soln, mm2.events["s_AMPA"]) 
    nptest.assert_array_almost_equal(gaba_soln, mm2.events["s_GABA"]) 
    nptest.assert_array_almost_equal(nmda_soln, mm2.events["s_NMDA"]) 
