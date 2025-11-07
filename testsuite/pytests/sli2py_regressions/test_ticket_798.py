# -*- coding: utf-8 -*-
#
# test_ticket_798.py
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


def test_quantal_stp_synapse_multithreaded():
    """
    Regression test for Ticket #798.

    Test ported from SLI regression test
    Ensure random number generation does not fail in multi-threaded simulation of quantal_stp_synapse.
    """
    nest.ResetKernel()
    nest.SetDefaults(
        "quantal_stp_synapse", {"U": 0.2, "u": 0.2, "tau_fac": 500.0, "tau_rec": 200.0, "weight": 1.0, "n": 5}
    )
    neurons = nest.Create("iaf_psc_exp", 10001)
    nest.SetStatus(neurons[0], {"I_e": 2000.0})
    nest.Connect(
        neurons[0], neurons[1:], conn_spec={"rule": "all_to_all"}, syn_spec={"synapse_model": "quantal_stp_synapse"}
    )
    nest.Simulate(100.0)
    # No assertion needed, test passes if no error is raised.
