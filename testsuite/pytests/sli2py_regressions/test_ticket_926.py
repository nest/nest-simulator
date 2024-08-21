# -*- coding: utf-8 -*-
#
# test_ticket_926.py
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
This ticket ensures that stdp_dopamine_synapse does not lead to a segfault, even if volume transmitter is not set.
"""

import nest
import pytest


def test_simulation_completes():
    nest.ResetKernel()
    spike_generator = nest.Create("spike_generator", {"spike_times": [5.0]})

    parrot_neuron = nest.Create("parrot_neuron")
    iaf_psc_neuron = nest.Create("iaf_psc_alpha")

    nest.Connect(spike_generator, parrot_neuron)

    # original SLI test uses passorfailbutnocrash_or_die; no clear way to check for a segfault in Python bindings,
    # therefore the assertion here is that the simulation completes with the expected error but no 'crash'
    with pytest.raises(nest.kernel.NESTError, match="No volume transmitter"):
        nest.Connect(parrot_neuron, iaf_psc_neuron, syn_spec="stdp_dopamine_synapse")
        nest.Simulate(10.0)
