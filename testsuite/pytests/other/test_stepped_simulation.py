# -*- coding: utf-8 -*-
#
# test_stepped_simulation.py
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
Test that multiple calls to ``Simulate`` give the same result as a single one.
"""

import nest


def build_net():
    nest.ResetKernel()

    nrn = nest.Create("iaf_psc_delta_ps")
    sgen = nest.Create("spike_generator", params={"spike_times": [0.4], "precise_times": False})
    srec = nest.Create("spike_recorder")

    nest.Connect(sgen, nrn, syn_spec={"weight": 15.0, "delay": 1.0})
    nest.Connect(nrn, srec)

    return srec


def test_stepped_simulation():
    """Ensure that stepped simulation results in the same as a single."""

    # Single simulation
    srec_single = build_net()
    nest.Simulate(3.0)
    spikes_single = srec_single.events["times"]

    # Stepped simulation
    srec_stepped = build_net()
    for _ in range(3):
        nest.Simulate(1.0)
    spikes_stepped = srec_stepped.events["times"]

    # Stepped simulation with RunManager
    srec_stepped_rm = build_net()
    with nest.RunManager():
        for _ in range(3):
            nest.Run(1.0)
    spikes_stepped_rm = srec_stepped_rm.events["times"]

    # Compare
    assert spikes_single == spikes_stepped
    assert spikes_single == spikes_stepped_rm
