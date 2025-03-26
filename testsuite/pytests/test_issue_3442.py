# -*- coding: utf-8 -*-
#
# test_issue_3442.py
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

"""
Tests that get_connections works if called when also using multithreading.
"""


@pytest.mark.parametrize("n_threads", [1, 2, 4])
def test_threaded_get_connections(n_threads):
    nest.ResetKernel()

    nest.total_num_virtual_procs = n_threads

    neuron = nest.Create("parrot_neuron")
    nest.Connect(neuron, neuron)

    nest.GetConnections()


test_threaded_get_connections(1)
