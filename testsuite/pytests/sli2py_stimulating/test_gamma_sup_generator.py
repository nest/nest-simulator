# -*- coding: utf-8 -*-
#
# test_gamma_sup_generator.py
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
Test parallel generation of trains

Creates a Gamma_Sup generator and sends spikes to spike recorder. Assert invariant
results for fixed VP number. This is a partial response to #551. May be adapted
to other generators.

"""

# TODO:: skip_if_not_threaded

import nest
import pytest


@pytest.fixture()
def total_vps():
    return 4


def foo(total_vps):
    nest.resolution = 0.1
    nest.total_num_virtual_procs = total_vps

    pg = nest.Create("gamma_sup_generator", {"rate": 100.})
    sr = nest.Create("spike_recorder")

    for i in range(total_vps):
        pn = nest.Create("parrot_neuron")
        nest.Connect(pg, pn)
        nest.Connect(pn, sr)

    nest.Simulate(10)


    events = sr.get("events")
    
