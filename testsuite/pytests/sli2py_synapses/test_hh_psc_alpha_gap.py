# -*- coding: utf-8 -*-
#
# test_hh_psc_alpha_gap.py
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
test_hh_psc_alpha_gap.sli is an overall test of the hh_psc_alpha_gap model connected
by gap_junction connection.

Two neurons of whom one receives an constant input current of 200 pA are connected
by gap_junction with an (unrealistic) high gap weight. The accurate functionality
of the gap_junction feature is tested by measuring the membrane potential of the
neuron without input current.

Although 0.1 cannot be represented in the IEEE double data type, it
is safe to simulate with a resolution (computation step size) of 0.1
ms because by default nest is built with a timebase enabling exact
representation of 0.1 ms.
"""

import nest
import pytest

pytestmark = pytest.mark.skipif_missing_gsl


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()
    nest.set(
        local_num_threads=1,
        resolution=0.1,
        use_wfr=True,
        wfr_tol=0.0001,
        wfr_interpolation_order=3,
        wfr_max_iterations=10,
        wfr_comm_interval=1.0,
    )


@pytest.fixture()
def prepare_voltmeter():
    n1, n2 = nest.Create("hh_psc_alpha_gap", 2)
    n1.set(I_e=200.0)

    vm = nest.Create("voltmeter")
    vm.set(time_in_steps=True, interval=nest.resolution)

    conn_rule = {"rule": "one_to_one", "make_symmetric": True}
    syn_dict = {"synapse_model": "gap_junction", "weight": 20.0}

    nest.Connect(n1, n2, conn_rule, syn_dict)
    nest.Connect(vm, n2, syn_spec={"weight": 1.0, "delay": nest.resolution})

    nest.Simulate(20)

    return vm


@pytest.fixture()
def reference_data():
    return [
        [1, -69.592],
        [2, -69.559],
        [3, -69.507],
        [4, -69.439],
        [5, -69.357],
        [6, -69.264],
        [7, -69.162],
        [8, -69.051],
        [9, -68.933],
        [10, -68.809],
        [11, -68.681],
        [12, -68.548],
        [13, -68.413],
        [14, -68.276],
        [15, -68.136],
        [117, -33.771],
        [118, -24.103],
        [119, 8.7117],
        [120, 62.019],
        [121, 39.042],
        [122, 27.485],
        [123, 18.856],
        [124, 11.201],
        [125, 3.6210],
        [126, -4.6956],
        [127, -15.006],
        [128, -29.464],
        [129, -49.786],
        [130, -71.323],
        [131, -83.787],
        [190, -71.023],
        [191, -70.833],
        [192, -70.647],
        [193, -70.466],
        [194, -70.289],
        [195, -70.116],
        [196, -69.948],
        [197, -69.783],
        [198, -69.622],
        [199, -69.464],
    ]


def test_hh_cond_beta_gap(prepare_voltmeter, reference_data):
    vm = prepare_voltmeter
    reference_data = dict(reference_data)

    events = vm.get("events")
    recorded_times = events["times"]
    recorded_vm = events["V_m"]

    actual_data = dict(zip(recorded_times, recorded_vm))
    actual_data = {k: actual_data[k] for k in reference_data.keys()}

    actual_data = list(actual_data.values())
    reference_data = list(reference_data.values())

    assert actual_data == pytest.approx(reference_data, rel=1e-4)
