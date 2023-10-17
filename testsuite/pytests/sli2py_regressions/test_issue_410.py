# -*- coding: utf-8 -*-
#
# test_issue_410.py
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
Regression test for Issue #410 (GitHub).
"""

import nest
import pytest

pytestmark = [pytest.mark.skipif_missing_gsl, pytest.mark.skipif_missing_threads]


def simulator(num_threads):
    """
    Simulate the system with provided number of threads and return weight.

    Based on simulation script provided in Issue #410.
    """

    nest.ResetKernel()

    nest.local_num_threads = num_threads

    stim1 = nest.Create("dc_generator", {"amplitude": 1500.0})
    stim2 = nest.Create("dc_generator", {"amplitude": 1000.0})
    nrn1 = nest.Create("iaf_psc_alpha", {"C_m": 100.0, "tau_m": 10.0})
    nrn2 = nest.Create("iaf_psc_alpha", {"C_m": 100.0, "tau_m": 10.0, "tau_minus": 10.0})
    dopa = nest.Create("iaf_cond_alpha", 100, {"V_reset": -70.0, "C_m": 80.0, "V_th": -60.0})
    vt = nest.Create("volume_transmitter")

    nest.CopyModel(
        "stdp_dopamine_synapse",
        "syn1",
        {
            "Wmax": 1000.0,
            "Wmin": 0.0,
            "tau_plus": 10.0,
            "A_minus": 0.05,
            "A_plus": 0.05,
            "b": 45.45,
            "tau_c": 1.0,
            "tau_n": 100.0,
            "volume_transmitter": vt,
        },
    )

    nest.Connect(stim1, nrn1, syn_spec={"weight": 10.0, "delay": 1.0})
    nest.Connect(stim2, dopa, syn_spec={"weight": 10.0, "delay": 1.0})
    nest.Connect(nrn1, nrn2, syn_spec={"synapse_model": "syn1", "weight": 500.0, "delay": 1.0})
    nest.Connect(dopa, vt)

    nest.Simulate(2000.0)

    conns = nest.GetConnections(source=nrn1, target=nrn2)
    weight = conns.get("weight")

    return weight


@pytest.fixture(scope="module")
def reference_weight():
    """
    Fixture that simulates reference.
    """

    ref_weight = simulator(1)
    return ref_weight


@pytest.mark.parametrize("num_threads", [2, 4, 8, 16, 32])
def test_multithreaded_volume_transmitter_and_stdp_dopamine_synapse(reference_weight, num_threads):
    """
    Test that ensures thread safety of volume transmitter.
    """

    weight = simulator(num_threads)
    assert weight == reference_weight
