# -*- coding: utf-8 -*-
#
# test_max_delay.py
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
Tests whether max_delay is updated correctly.
"""

import nest
import pytest


def test_max_delay():
    nest.ResetKernel()

    delay = 314.1  # multiple of dt
    neuron1 = nest.Create("iaf_psc_alpha")
    neuron2 = nest.Create("iaf_psc_alpha")
    nest.Connect(neuron1, neuron2, syn_spec={"delay": delay})

    assert nest.GetKernelStatus("max_delay") == delay
