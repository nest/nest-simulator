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

r"""
Test that per-thread nodes vectors are updated.

1. Create nodes driven by internal current, simulate, check V_m changes.
2. Add more nodes, reset old nodes, simulate again, check V_m changes for all.
3. Freeze half the nodes, simulate, check their V_m is constant.
4. Thaw nodes, simulate, check V_m changes again.

"""

import numpy as np
import pytest
import nest

pytestmark = pytest.mark.skipif_missing_threads()

NUM_THREADS = 4
BLOCKSIZE = 11   # number of neurons per Create, not multiple of NUM_THREADS
SIMTIME = 10
I_E = -50
VM_REF = nest.GetDefaults('iaf_psc_alpha', 'V_m')


@pytest.fixture(scope="module")
def prepare_kernel():
    nest.ResetKernel()
    nest.local_num_threads = 4


@pytest.fixture(scope="module")
def neurons_a(prepare_kernel):
    """
    Set up network and simulate it step by step.
    """

    return nest.Create('iaf_psc_alpha', n=BLOCKSIZE, params={'I_e': I_E})


@pytest.fixture(scope="module")
def neurons_b(neurons_a):
    return nest.Create('iaf_psc_alpha', n=BLOCKSIZE, params={'I_e': I_E})


@pytest.fixture(scope="module")
def sim_a(neurons_a):
    nest.Simulate(SIMTIME)


@pytest.fixture(scope="module")
def sim_a_b(neurons_a, neurons_b, sim_a):
    (neurons_a + neurons_b).V_m = VM_REF
    nest.Simulate(SIMTIME)


@pytest.fixture(scope="module")
def sim_frozen(neurons_a, neurons_b, sim_a_b):
    (neurons_a + neurons_b).V_m = VM_REF
    neurons_a.frozen = True
    nest.Simulate(SIMTIME)


@pytest.fixture(scope="module")
def sim_thawed(neurons_a, neurons_b, sim_frozen):
    (neurons_a + neurons_b).V_m = VM_REF
    neurons_a.frozen = False
    nest.Simulate(SIMTIME)


def test_vm_changes(neurons_a, sim_a):
    assert all(np.array(neurons_a.V_m) < VM_REF)


def test_vm_changes_more_neurons(neurons_a, neurons_b, sim_a_b):
    assert all(np.array((neurons_a + neurons_b).V_m) < VM_REF)


def test_vm_frozen(neurons_a, neurons_b, sim_frozen):
    assert all(np.array(neurons_a.V_m) == VM_REF) and all(np.array(neurons_b.V_m) < VM_REF)


def test_vm_thawed(neurons_a, neurons_b, sim_thawed):
    assert all(np.array((neurons_a + neurons_b).V_m) < VM_REF)
