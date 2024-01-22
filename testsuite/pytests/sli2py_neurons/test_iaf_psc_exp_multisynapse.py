# -*- coding: utf-8 -*-
#
# test_iaf_psc_exp_multisynapse.py
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
Test ``iaf_psc_exp_multisynapse`` recordables and simulated PSCs against expectation.
"""


import nest
import numpy as np
import numpy.testing as nptest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def exp_psc_fn(t, tau_syn):
    vals = np.zeros_like(t)
    nonzero_inds = t > 0.0
    vals[nonzero_inds] = np.exp(-t[nonzero_inds] / tau_syn)
    return vals


def exp_psc_voltage_response(t, tau_syn, tau_m, C_m, w):
    vals = np.zeros_like(t)
    nonzero_inds = t > 0.0
    delta_e = np.exp(-t[nonzero_inds] / tau_m) - np.exp(-t[nonzero_inds] / tau_syn)
    vals[nonzero_inds] = w / (C_m * (1.0 / tau_syn - 1.0 / tau_m)) * delta_e
    return vals


def test_set_synaptic_time_constants():
    """Tests that synaptic time constants can be set correctly"""
    taus = [2.0, 20.0, 60.0, 100.0]
    nrn = nest.Create("iaf_psc_exp_multisynapse")
    nrn.set(tau_syn=taus)
    nptest.assert_array_almost_equal(nrn.get("tau_syn"), taus)


def test_simulation_against_analytical_solution():
    """
    Test simulated PSCs against analytical expectation.

    This test checks that the integration of the exponential currents of inputs
    from multiple different synaptic ports are the same as the analytical solution.
    """

    tau_syns = [2.0, 20.0, 60.0, 100.0]
    delays = [7.0, 5.0, 2.0, 1.0]
    weights = [30.0, 50.0, 20.0, 10.0]
    C_m = 250.0
    tau_m = 15.0
    spike_time = 0.1
    simtime = 8.0
    dt = 0.1

    nest.set(resolution=dt)

    nrn = nest.Create(
        "iaf_psc_exp_multisynapse",
        params={
            "C_m": C_m,
            "E_L": 0.0,
            "V_m": 0.0,
            "V_th": 1500.0,
            "I_e": 0.0,
            "tau_m": tau_m,
            "tau_syn": tau_syns,
        },
    )

    sg = nest.Create("spike_generator", params={"spike_times": [spike_time]})

    for syn_idx, (delay, weight) in enumerate(zip(delays, weights)):
        syn_spec = {
            "synapse_model": "static_synapse",
            "delay": delay,
            "weight": weight,
            "receptor_type": syn_idx + 1,
        }
        nest.Connect(sg, nrn, conn_spec="one_to_one", syn_spec=syn_spec)

    mm = nest.Create(
        "multimeter",
        params={"record_from": ["I_syn_1", "I_syn_2", "I_syn_3", "I_syn_4", "V_m", "I_syn"], "interval": dt},
    )

    nest.Connect(mm, nrn, syn_spec={"delay": 0.1})
    nest.Simulate(simtime)
    times = mm.get("events", "times")

    I_syns_analytical = []
    V_m_analytical = np.zeros_like(times)
    for weight, delay, tau_s in zip(weights, delays, tau_syns):
        I_syns_analytical.append(exp_psc_fn(times - delay - spike_time, tau_s) * weight)
        V_m_analytical += exp_psc_voltage_response(times - delay - spike_time, tau_s, tau_m, C_m, weight)

    for idx, I_syn_analytical in enumerate(I_syns_analytical):
        nptest.assert_array_almost_equal(mm.get("events", f"I_syn_{idx+1}"), I_syn_analytical)
    nptest.assert_array_almost_equal(mm.get("events", "V_m"), V_m_analytical)


# The following tests address #800
# - Test that the default recordables are V_m, w and I_syn_1
# - Test that the recordable I_syn's change when changing the number of receptor ports


def test_default_recordables():
    nrn = nest.Create("iaf_psc_exp_multisynapse")
    recordables = nrn.get("recordables")
    assert len(recordables) == 3
    assert "I_syn" in recordables
    assert "I_syn_1" in recordables
    assert "V_m" in recordables


def test_resize_recordables():
    """
    Test resizing of recordables.

    This test ensures that recordables are updated correctly when the number
    of synaptic ports are changed.
    """

    tau_syn1 = [5.0, 1.0, 25.0]
    tau_syn2 = [5.0, 1.0]
    tau_syn3 = [5.0, 1.0, 25.0, 50.0]

    nrn = nest.Create("iaf_psc_alpha_multisynapse", params={"tau_syn": tau_syn1})
    assert len(nrn.recordables) == 5

    nrn.set(tau_syn=tau_syn2)
    assert len(nrn.recordables) == 4

    nrn.set(tau_syn=tau_syn3)
    assert len(nrn.recordables) == 6
