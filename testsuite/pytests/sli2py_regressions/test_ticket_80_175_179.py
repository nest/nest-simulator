# -*- coding: utf-8 -*-
#
# test_ticket_80_175_179.py
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
   This test verifies that:
   voltmeter and spike_recorder record identical signals from iaf_psc_alpha
   driven by internal dc independent of how simulation time is blocked
   and how elements are placed in script. Additionally, two spike generators,
   placed before and after the recorders, emit the same spike trains that
   are expected from the neurons for the given parameters.

   Protocol:
   1. One iaf_psc_alpha created before voltmeters and spike recorders, one after.
   2. Devices recording from both neurons.
   3. Neurons driven by internal dc current.
   4. Resolution fixed, but simulation time subdivided in different ways.
   5. Test that all devices yield identical results under all conditions.
"""

import nest
import numpy as np
import pytest


@pytest.fixture
def setup():
    vm_params = {"origin": 0.0, "start": 0.0, "stop": 100.0, "interval": nest.resolution}
    sr_params = {"origin": 0.0, "start": 0.0, "stop": 100.0, "time_in_steps": True}
    sg_params = {
        "spike_times": [4.8, 11.6, 18.4, 25.2, 32.0, 38.8, 45.6, 52.4, 59.2, 66.0, 72.8, 79.6, 86.4, 93.2, 100.0]}
    iaf_params = {"I_e": 1000.0}
    sim_blocks = [0.1, 0.3, 0.5, 0.7, 1.0, 1.3, 1.5, 1.7, 110.0]

    sim_time = vm_params["stop"] + 2.0

    for block in sim_blocks:
        nest.ResetKernel()
        nest.resolution = 0.1

        nest.SetDefaults("iaf_psc_alpha", iaf_params)
        nest.SetDefaults("voltmeter", vm_params)
        nest.SetDefaults("spike_recorder", sr_params)
        nest.SetDefaults("spike_generator", sg_params)

        spike_gen_pre = nest.Create("spike_generator")
        iaf_psc_alpha_pre = nest.Create("iaf_psc_alpha")

        spike_recorders = [nest.Create("spike_recorder") for _ in range(4)]
        voltmeters = [nest.Create("voltmeter") for _ in range(2)]

        spike_gen_post = nest.Create("spike_generator")
        iaf_psc_alpha_post = nest.Create("iaf_psc_alpha")

        nest.Connect(iaf_psc_alpha_pre, spike_recorders[0])
        nest.Connect(iaf_psc_alpha_post, spike_recorders[1])
        nest.Connect(spike_gen_pre, spike_recorders[2])
        nest.Connect(spike_gen_post, spike_recorders[3])

        nest.Connect(voltmeters[0], iaf_psc_alpha_pre)
        nest.Connect(voltmeters[1], iaf_psc_alpha_post)

        while sim_time >= nest.biological_time:
            nest.Simulate(block)

        sr_timings = np.array([sr.get('events')['times'] for sr in spike_recorders])
        vm_timings = np.array([vm.get('events')['V_m'] for vm in voltmeters])

        return sr_timings, vm_timings


def test_sr_produce_same_output(setup):
    sr_timings, _ = setup
    assert np.all((sr_timings[0] == sr_timings).all(axis=1))


def test_vm_produce_same_output(setup):
    _, vm_timings = setup
    assert np.all((vm_timings[0] == vm_timings).all(axis=1))
