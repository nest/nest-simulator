# -*- coding: utf-8 -*-
#
# test_delay_exchange.py
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


def test_delay_exchange():
    """
    Confirm that creating a single connection on a single rank will set delay extrema on all ranks.

    @note This test must be run on multiple MPI ranks to be meaningful.
    """

    assert nest.num_processes == 4

    min_delay = 0.5
    max_delay = 2.5

    n = nest.Create("parrot_neuron")
    nest.Connect(n, n, "one_to_one", {"synapse_model": "static_synapse", "delay": min_delay})
    nest.Connect(n, n, "one_to_one", {"synapse_model": "static_synapse", "delay": max_delay})

    # Accessing kernel attributes forces GetKernelStatus with exchange of delay info
    assert nest.min_delay == min_delay
    assert nest.max_delay == max_delay
