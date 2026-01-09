# -*- coding: utf-8 -*-
#
# test_axonal_delay_connectivity_changed.py
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


@pytest.mark.parametrize("conn1, conn2", [[0.0, 0.0], [1.0, 0.0], [0.0, 1.0], [1.0, 1.0]])
def test_axonal_delay_connectivity_changed(conn1, conn2):
    """
    Test that changing connectivity during simulation throws an exception when using axonal delays.
    """
    nest.ResetKernel()

    nest.SetKernelStatus({"min_delay": 1.0, "max_delay": 2.0})

    neuron = nest.Create("iaf_psc_alpha")

    nest.Connect(neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay", "axonal_delay": conn1})

    nest.Simulate(nest.resolution)

    nest.Connect(neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay", "axonal_delay": conn2})

    if conn1 > 0.0 or conn2 > 0.0:
        with pytest.raises(nest.kernel.NESTError):
            nest.Simulate(nest.resolution)
    else:
        nest.Simulate(nest.resolution)
