# -*- coding: utf-8 -*-
#
# test_ticket_772.py
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


@pytest.mark.parametrize(
    "rule, n_tgts, weights, delays",
    [("all_to_all", 1, 1, 4), ("all_to_all", 1, [[1, 1]], [[4, 6]]), ("one_to_one", 2, [1, 1], [4, 6])],
)
def test_integer_weights_and_delays_do_not_crash(rule, n_tgts, weights, delays):
    """
    Regression test for Ticket #772.

    Test ported from SLI regression test
    Ensure that integer values for weight and delay do not crash NEST in Connect.
    """

    n = nest.Create("parrot_neuron", 2)
    nest.Connect(
        n,
        n[:n_tgts],
        conn_spec={"rule": rule},
        syn_spec={"synapse_model": "static_synapse", "weight": weights, "delay": delays},
    )
    # No assertion needed, test passes if no error is raised.
