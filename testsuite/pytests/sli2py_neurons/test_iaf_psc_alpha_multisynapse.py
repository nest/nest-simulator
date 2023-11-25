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
Test ``iaf_psc_alpha_multisynapse`` recordables and simulated PSCs against expectation.
"""


import nest
import numpy as np
import numpy.testing as nptest
import pytest
from scipy.linalg import expm


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def alpha_fn(t, tau_syn):
    vals = np.zeros_like(t)
    nonzero_inds = t > 0.0
    vals[nonzero_inds] = np.e / tau_syn * t[nonzero_inds] * np.exp(-t[nonzero_inds] / tau_syn)
    return vals


def test_I_syn_1_in_recordables():
    """Test that ``I_syn_1`` is in the list of recordables."""

    nrn = nest.Create("iaf_psc_alpha_multisynapse")
    assert "I_syn_1" in nrn.get("recordables")


def alpha_psc_voltage_response(t, tau_syn, tau_m, C_m, w):
    vals = np.zeros_like(t)
    nonzero_inds = t > 0.0
    A = np.array([[-1.0 / tau_syn, 0.0, 0.0], [1.0, -1.0 / tau_syn, 0.0], [0.0, 1.0 / C_m, -1.0 / tau_m]])

    expAt = expm(A[None, ...] * t[nonzero_inds, None, None])  # shape (t, 3, 3)
    vals[nonzero_inds] = expAt[:, 2, 0] * w * np.e / tau_syn  # first two state variables are 0
    return vals


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


def test_simulation_against_analytical_soln():
    """
    Test simulated PSCs against analytical expectation.

    This test checks that the integration of the alpha-shaped currents of inputs
    from multiple different synaptic ports are the same as the analytical solution.
    """

    tau_syns = [2.0, 20.0, 60.0, 100.0]
    delays = [7.0, 5.0, 2.0, 1.0]
    weights = [30.0, 50.0, 20.0, 10.0]
    C_m = 250.0
    tau_m = 15.0
    spike_time = 1.0
    simtime = 100.0
    dt = 1.0

    nest.set(resolution=dt)

    nrn = nest.Create(
        "iaf_psc_alpha_multisynapse",
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

    nest.Connect(mm, nrn)
    nest.Simulate(simtime)
    times = mm.get("events", "times")

    I_syns_analytical = []
    V_m_analytical = np.zeros_like(times)
    for weight, delay, tau_s in zip(weights, delays, tau_syns):
        I_syns_analytical.append(alpha_fn(times - delay - spike_time, tau_s) * weight)
        V_m_analytical += alpha_psc_voltage_response(times - delay - spike_time, tau_s, tau_m, C_m, weight)

    for idx, I_syn_analytical in enumerate(I_syns_analytical):
        nptest.assert_array_almost_equal(mm.get("events", f"I_syn_{idx+1}"), I_syn_analytical)

    nptest.assert_array_almost_equal(mm.get("events", "V_m"), V_m_analytical)
