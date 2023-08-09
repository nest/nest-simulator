# -*- coding: utf-8 -*-
#
# test_iaf_ps_psp_poisson_accuracy.py
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
import pytest
import math
from math import exp

# Global parameters
T = 6.
tau_syn = 0.3
tau_m = 10.
C_m = 250.
weight = 65.
delay = 1.0

neuron_params = {"E_L": 0.,
                 "V_m": 0.,
                 "V_th": 15.,
                 "I_e": 0.,
                 "tau_m": tau_m,
                 "tau_syn_ex": tau_syn,
                 "tau_syn_in": tau_syn,
                 "C_m": C_m}


def alpha_fn(t):
    prefactor = weight * math.e / (tau_syn * C_m)
    t1 = (exp(-t / tau_m) - exp(-t / tau_syn)) / (1 / tau_syn - 1 / tau_m)**2
    t2 = t * exp(-t / tau_syn) / (1 / tau_syn - 1 / tau_m)
    return prefactor * (t1 - t2)
 

def spiketrain_response(spiketrain):
    response = 0.
    for sp in spiketrain:
        t = T - delay - sp
        response += alpha_fn(t)
    return response

@pytest.mark.parametrize("h", [(i) for i in [-4]])#range(-10, 1, 2)])
def test_poisson_spikes_different_stepsizes(h):
    nest.ResetKernel()

    nest.set(tics_per_ms=2**10, resolution=2**h)

    pg = nest.Create("poisson_generator")
    pg.set(rate = 1000)
#    pg = nest.Create("spike_generator")
#    pg.set(spike_times = [3.125])

    parrot = nest.Create("parrot_neuron")
    neuron = nest.Create("iaf_psc_alpha_ps")
    neuron.set(neuron_params)

    sr = nest.Create("spike_recorder")

    mm = nest.Create("multimeter")
    mm.set(record_from=["V_m"])
    mm.set(interval=2**h)

    nest.Connect(pg, parrot)
    nest.Connect(parrot, neuron, syn_spec={"weight": weight, "delay": delay})
    nest.Connect(parrot, sr)

    nest.Simulate(T)

    spiketrain = sr.get("events", "times")
    analytical_u = spiketrain_response(spiketrain) 
    print(spiketrain)
    u = neuron.get("V_m")
    assert u == pytest.approx(analytical_u)

