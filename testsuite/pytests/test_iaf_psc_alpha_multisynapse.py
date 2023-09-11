# -*- coding: utf-8 -*-
#
# test_iaf_psc_alpha_multisynapse.py
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
Test that iaf_psc_alpha_multisynapse recordables work correctly, and compare integration to analytical solution

This module contains three tests.
The first tests that I_syn_1 is in the list of recordables.
The second tests that the recordables are updated correctly when the number
of synaptic ports are changed.
The third tests that the integration of the alpha-shaped currents of inputs
from multiple different synaptic ports gives the correct solution, computed
analytically.
"""


import nest
import numpy as np
import numpy.testing as nptest
import pytest


tau_syn = [2., 20., 60., 100.]
delays = [100., 200., 500., 1200.]
spike_time = 100.0
weight = 1.
dt = 0.1
C_m = 250.
tau_m = 15.
spike_time = 10.0
simtime = 2500.0

neuron_params = {
    "E_L": 0.0,
    "V_m": 0.0,
    "V_th": 1500.0,
    "I_e": 0.0,
    "tau_m": tau_m,
    "tau_syn": tau_syn,
    "C_m": C_m
    }


def test_I_syn_1_in_recordables():
    nrn = nest.Create("iaf_psc_alpha_multisynapse")
    assert "I_syn_1" in nrn.get("recordables")


def test_resize_recordables():
    tau_syn1 = [5.0, 1.0, 25.0]
    tau_syn2 = [5.0, 1.0]
    tau_syn3 = [5.0, 1.0, 25.0, 50.0]

    nrn = nest.Create("iaf_psc_alpha_multisynapse", params={"tau_syn": tau_syn1})
    recordables1 = nrn.get("recordables")

    nrn.set(tau_syn=tau_syn2)
    recordables2 = nrn.get("recordables")

    nrn.set(tau_syn=tau_syn3)
    recordables3 = nrn.get("recordables")

    assert len(recordables1) == 5
    assert len(recordables2) == 4
    assert len(recordables3) == 6


def alpha_fn(t, tau_syn):
    vals = np.zeros_like(t)
    zero_inds = t <= 0.0
    nonzero_inds = ~zero_inds
    vals[nonzero_inds] = np.e / tau_syn * t[nonzero_inds] * np.exp(-t[nonzero_inds] / tau_syn)
    return vals


def test_simulation_against_analytical_soln():
    nest.ResetKernel()
    nrn = nest.Create("iaf_psc_alpha_multisynapse", params=neuron_params)
    sg = nest.Create("spike_generator", params={"spike_times": [spike_time]})
    conn_spec = "one_to_one"
    for i, syn_id in enumerate(range(1, 5)):
        syn_spec = {"synapse_model": "static_synapse",
                    "delay": delays[i],
                    "weight": weight,
                    "receptor_type": syn_id}

        nest.Connect(sg, nrn, conn_spec=conn_spec, syn_spec=syn_spec)

    mm = nest.Create("multimeter",
                     params={"record_from": ["I_syn_1", "I_syn_2", "I_syn_3", "I_syn_4"]})

    nest.Connect(mm, nrn)
    nest.Simulate(simtime)
    times = mm.get("events", "times")
    I_syn = np.sum([mm.get("events", f"I_syn_{i}") for i in range(1, 5)], axis=0)

    I_syn_analytical = np.zeros_like(times, dtype=np.float64)
    for i in range(4):
        I_syn_analytical += alpha_fn(times - delays[i] - spike_time, tau_syn[i])

    nptest.assert_array_almost_equal(I_syn, I_syn_analytical)
