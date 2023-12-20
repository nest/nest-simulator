# -*- coding: utf-8 -*-
#
# test_add_freeze_thaw.py
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
Test that per-thread nodes vectors are updated.
"""

import nest
import numpy as np
import pytest


@pytest.mark.skipif_missing_threads
def test_add_freeze_thaw():
    """
    The following simulation steps and checks are performed in order:

      1. Create nodes driven by internal current, simulate, check V_m changes.
      2. Add more nodes, reset old nodes, simulate again, check V_m changes for all.
      3. Freeze half the nodes, simulate, check their V_m is constant.
      4. Thaw nodes, simulate, check V_m changes again.

    Because the order of simulation steps and tests is important, implement as single test.
    """

    nest.ResetKernel()
    nest.local_num_threads = 4

    neuron_model = "iaf_psc_alpha"
    neurons_per_pop = 11  # not multiple of number of threads
    neuron_params = {"I_e": -50.0}
    sim_time = 10.0

    vm_ref = nest.GetDefaults(neuron_model, "V_m")

    # Test that neurons in population a hyperpolarize
    pop_a = nest.Create(neuron_model, n=neurons_per_pop, params=neuron_params)
    nest.Simulate(sim_time)
    assert all(np.array(pop_a.V_m) < vm_ref)

    # Test that neurons in population a and b hyperpolarize
    pop_b = nest.Create(neuron_model, n=neurons_per_pop, params=neuron_params)
    (pop_a + pop_b).V_m = vm_ref
    nest.Simulate(sim_time)
    assert all(np.array((pop_a + pop_b).V_m) < vm_ref)

    # Test that neurons in population a do not hyperpolarize when frozen
    (pop_a + pop_b).V_m = vm_ref
    pop_a.frozen = True
    nest.Simulate(sim_time)
    assert all(np.array(pop_a.V_m) == vm_ref) and all(np.array(pop_b.V_m) < vm_ref)

    # Test that neurons in population a hyperpolarize when no longer frozen
    (pop_a + pop_b).V_m = vm_ref
    pop_a.frozen = False
    nest.Simulate(sim_time)
    assert all(np.array((pop_a + pop_b).V_m) < vm_ref)
