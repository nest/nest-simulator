# -*- coding: utf-8 -*-
#
# test_min_max_delay.py
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
Tests whether max_delay and min_delay is updated correctly.
"""


import nest
import pytest


@pytest.fixture(scope="module")
def network():
    nest.ResetKernel()

    max_delay = 314.1  # multiple of dt
    min_delay = 2.5
    neuron1 = nest.Create("iaf_psc_alpha")
    neuron2 = nest.Create("iaf_psc_alpha")
    neuron3 = nest.Create("iaf_psc_alpha")
    nest.Connect(neuron1, neuron2, syn_spec={"delay": min_delay})
    nest.Connect(neuron2, neuron3, syn_spec={"delay": max_delay})
    nest.Connect(neuron3, neuron1, syn_spec={"delay": max_delay - min_delay})
    return {"min_delay": min_delay, "max_delay": max_delay}


def test_min_delay(network):
    assert nest.GetKernelStatus("min_delay") == network["min_delay"]


def test_max_delay(network):
    assert nest.GetKernelStatus("max_delay") == network["max_delay"]
