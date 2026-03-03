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
Regression test for ticket #80, #175 and #179.

This set of test verifies that ``voltmeter`` and ``spike_recorder`` record
identical signals from ``iaf_psc_alpha`` driven by internal DC independent
of how simulation time is blocked and how elements are placed in script.
Additionally, two spike generators, placed before and after the recorders,
emit the same spike trains that are expected from the neurons for the given
parameters.
"""

import nest
import numpy.testing as nptest
import pytest

vm_stop = 100.0
total_sim_time = vm_stop + 2.0


def build_net():
    """
    Function for setting up the system.

    The system is set up and simulated according to the following protocol:

        1. One ``iaf_psc_alpha`` created before voltmeters and spike recorders,
        one after.
        2. Devices recording from both neurons.
        3. Neurons driven by internal DC current.
        4. Resolution fixed, but simulation time subdivided in different ways.

    The function returns the ``spike_recorder`` and ``voltmeter`` devices.
    """

    nest.ResetKernel()
    nest.resolution = 0.1

    vm_params = {
        "origin": 0.0,
        "start": 0.0,
        "stop": vm_stop,
        "interval": 0.1,
    }
    sr_params = {"origin": 0.0, "start": 0.0, "stop": 100.0, "time_in_steps": True}
    sg_params = {
        "spike_times": [
            4.8,
            11.6,
            18.4,
            25.2,
            32.0,
            38.8,
            45.6,
            52.4,
            59.2,
            66.0,
            72.8,
            79.6,
            86.4,
            93.2,
            100.0,
        ]
    }
    iaf_params = {"I_e": 1000.0}

    sg_pre = nest.Create("spike_generator", 1, sg_params)
    nrn_pre = nest.Create("iaf_psc_alpha", 1, iaf_params)
    srs = nest.Create("spike_recorder", 4, sr_params)
    vms = nest.Create("voltmeter", 2, vm_params)
    sg_post = nest.Create("spike_generator", 1, sg_params)
    nrn_post = nest.Create("iaf_psc_alpha", 1, iaf_params)

    nest.Connect(nrn_pre, srs[0])
    nest.Connect(nrn_post, srs[1])
    nest.Connect(sg_pre, srs[2])
    nest.Connect(sg_post, srs[3])
    nest.Connect(vms[0], nrn_pre)
    nest.Connect(vms[1], nrn_post)

    return srs, vms


@pytest.fixture(scope="module")
def reference_run():
    """
    Fixture for running the reference simulation, full simulation time in one go.
    """

    srs, vms = build_net()
    nest.Simulate(total_sim_time)
    srs_reference = [event["times"] for event in srs.events]
    vms_reference = [event["V_m"] for event in vms.events]

    return srs_reference, vms_reference


def test_reference_recordings_identical(reference_run):
    """
    Consistency test to ensure that the reference recordings are identical.
    """

    srs_reference, vms_reference = reference_run

    for srec in srs_reference[1:]:
        nptest.assert_array_equal(srs_reference[0], srec)

    nptest.assert_array_equal(vms_reference[0], vms_reference[1])


@pytest.mark.parametrize("t_block", [0.1, 0.3, 0.5, 0.7, 1.0, 1.3, 1.5, 1.7, 110.0])
def test_vm_and_sr_produce_same_output(t_block, reference_run):
    """
    Test that the ``voltmeter`` and ``spike_recorder`` yield identical results independent simulation time blocking.
    """

    srs, vms = build_net()

    with nest.RunManager():
        while nest.biological_time < total_sim_time:
            nest.Run(t_block)

    srs_times = [event["times"] for event in srs.events]
    vms_recs = [event["V_m"] for event in vms.events]

    srs_reference, vms_reference = reference_run

    # Test that recorders give identical results independent of simulation time blocking
    nptest.assert_array_equal(srs_reference, srs_times)
    nptest.assert_array_equal(vms_reference, vms_recs)
