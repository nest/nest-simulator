# -*- coding: utf-8 -*-
#
# test_issue_832.py
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

"""
Regression test for Issue #832 (GitHub).

Test ported from SLI regression test.
Ensure that deleting auxiliary connections in gap-junction networks leaves the primary_end marker consistent.

Author: Jan Hahne, October 2017
"""


REFERENCE_DATA = [
    (1, -69.592),
    (2, -69.559),
    (3, -69.507),
    (4, -69.439),
    (5, -69.357),
    (6, -69.264),
    (7, -69.162),
    (8, -69.051),
    (9, -68.933),
    (10, -68.809),
    (11, -68.681),
    (12, -68.548),
    (13, -68.413),
    (14, -68.276),
    (15, -68.136),
    (117, -33.771),
    (118, -24.103),
    (119, 8.7117),
    (120, 62.019),
    (121, 39.042),
    (122, 27.485),
    (123, 18.856),
    (124, 11.201),
    (125, 3.6210),
    (126, -4.6956),
    (127, -15.006),
    (128, -29.464),
    (129, -49.786),
    (130, -71.323),
    (131, -83.787),
    (190, -71.023),
    (191, -70.833),
    (192, -70.647),
    (193, -70.466),
    (194, -70.289),
    (195, -70.116),
    (196, -69.948),
    (197, -69.783),
    (198, -69.622),
    (199, -69.464),
]


@pytest.mark.skipif_missing_gsl
def test_issue_832_gap_junction_marker_stability():
    """
    Ensure gap junctions remain valid when additional primary connections are created and deleted.
    """

    nest.ResetKernel()
    nest.SetKernelStatus(
        {
            "local_num_threads": 1,
            "resolution": 0.1,
            "use_wfr": True,
            "wfr_tol": 0.0001,
            "wfr_interpolation_order": 3,
            "wfr_max_iterations": 10,
            "wfr_comm_interval": 1.0,
        }
    )

    neuron1 = nest.Create("hh_psc_alpha_gap")
    neuron2 = nest.Create("hh_psc_alpha_gap")

    nest.CopyModel("static_synapse", "static1")
    nest.CopyModel("static_synapse", "static2")

    nest.SetStatus(neuron1, {"I_e": 200.0})

    voltmeter = nest.Create("voltmeter", params={"time_in_steps": True, "interval": 0.1})

    nest.Connect(
        neuron1,
        neuron2,
        conn_spec={"rule": "one_to_one", "make_symmetric": True},
        syn_spec={"synapse_model": "gap_junction", "weight": 20.0},
    )

    nest.Connect(voltmeter, neuron2)

    nest.Connect(
        neuron1,
        neuron2,
        conn_spec={"rule": "one_to_one"},
        syn_spec={"synapse_model": "static1", "weight": 0.0},
    )
    nest.Connect(
        neuron1,
        neuron2,
        conn_spec={"rule": "one_to_one"},
        syn_spec={"synapse_model": "static2", "weight": 0.0},
    )

    nest.Disconnect(
        neuron1,
        neuron2,
        conn_spec={"rule": "all_to_all"},
        syn_spec={"synapse_model": "static1"},
    )

    nest.Simulate(20.0)

    events = nest.GetStatus(voltmeter, "events")[0]
    times = events["times"]
    voltages = events["V_m"]

    reference_map = {time: value for time, value in REFERENCE_DATA}
    mismatches = []
    for ref_time, expected_value in reference_map.items():
        matching_indices = [idx for idx, t in enumerate(times) if abs(t - ref_time) < 1e-6]
        if not matching_indices:
            mismatches.append((ref_time, "missing"))
            continue

        measured = voltages[matching_indices[0]]
        if not pytest.approx(expected_value, abs=1e-3) == measured:
            mismatches.append((ref_time, measured, expected_value))

    assert not mismatches, mismatches
