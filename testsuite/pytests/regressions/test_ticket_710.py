# -*- coding: utf-8 -*-
#
# test_ticket_710.py
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


def test_static_synapse_hpc_connects_to_spike_recorder():
    """
    Regression test for Ticket #710.
    Test ported from SLI regression test
    Ensure that static_synapse_hpc can be used to connect a neuron to a spike recorder.
    """
    nest.ResetKernel()
    neuron = nest.Create("iaf_psc_alpha")
    sr = nest.Create("spike_recorder")
    nest.Connect(neuron, sr, syn_spec={"synapse_model": "static_synapse_hpc"})
    # No assertion needed, test passes if no error is raised.
