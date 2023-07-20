# -*- coding: utf-8 -*-
#
# test_binary.py
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
import numpy as np
import pytest


class TestBinary:
    r"""Test of binary neuron models.

    Test basic communication patterns employed by binary neurons"""

    @pytest.mark.parametrize("neuron_model", ["ginzburg_neuron", "mcculloch_pitts_neuron", "erfc_neuron"])
    def test_binary(self, neuron_model):
        r"""Test, whether the communication mechanism for binary neurons works. Two spikes indicate an up-transition,
        a single spike indicate a down transition."""

        h = 0.1  # resolution [ms]

        nest.ResetKernel()
        nest.resolution = h

        print(f'Testing model "{neuron_model}"')

        # check, if double spikes are correctly interpreted as up transition and single spikes are interpreted as
        # down transition

        # expected read out at target neuron
        expected = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0]

        nrn = nest.Create(neuron_model)
        sg = nest.Create("spike_generator")
        sg.spike_times = [10.0, 10.0, 15.0]
        nest.Connect(sg, nrn)
        multi = nest.Create("multimeter", {"record_from": ["h"]})
        nest.Connect(multi, nrn)

        nest.Simulate(20.0)

        h_recorded = multi.events["h"]
        np.testing.assert_allclose(expected, h_recorded)

    @pytest.mark.parametrize("neuron_model", ["ginzburg_neuron", "mcculloch_pitts_neuron", "erfc_neuron"])
    def test_binary_neuron_state_change(self, neuron_model):
        r"""check, if binary_neuron correctly transmits its state change"""

        h = 0.1  # resolution [ms]

        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": h})

        nrn1 = nest.Create(neuron_model)
        sr = nest.Create("spike_recorder")

        # set threshold so that neuron makes transition to 1
        nrn1.theta = -10.0
        nrn1.tau_m = 1.0

        nest.Connect(nrn1, sr)

        nest.Simulate(100.0)

        # should have two events
        assert len(sr.events["senders"]) == 2

        # both events should be at same time point
        assert len(np.unique(sr.events["times"])) == 1

        assert len(sr.events["senders"]) == 2

        # set threshold so that neuron makes transition to 0
        nrn1.theta = 10.0

        nest.Simulate(100.0)

        # should have one more event now
        assert len(sr.events["senders"]) == 3

    def test_connecting_binary(self):
        binary = nest.Create("mcculloch_pitts_neuron")
        spiking = nest.Create("iaf_psc_alpha")
        sg = nest.Create("spike_generator")
        sr = nest.Create("spike_recorder")

        # check if connecting a binary to a spiking neuron throws exception
        with pytest.raises(nest.kernel.NESTError):
            nest.Connect(binary, spiking)

        # check if connecting a spiking neuron to a binary throws exception
        with pytest.raises(nest.kernel.NESTError):
            nest.Connect(spiking, binary)

        # check if connecting a binary or a spiking neuron to general device works
        nest.Connect(sg, spiking)
        nest.Connect(sg, binary)
        nest.Connect(spiking, sr)
        nest.Connect(binary, sr)

    def test_keep_source_table(self):
        r"""Check if simulating with two connected binary neurons when keep_source_table is set to false throws
        exception"""
        nest.ResetKernel()
        nest.SetKernelStatus({"keep_source_table": False})
        ginzburg = nest.Create("ginzburg_neuron")
        mcculloch = nest.Create("mcculloch_pitts_neuron")

        nest.Connect(ginzburg, mcculloch)

        with pytest.raises(nest.kernel.NESTError):
            nest.Simulate(100.0)
