"""
Test that per-thread nodes vectors are updated.

1. Create nodes driven by internal current, simulate, check V_m changes.
2. Add more nodes, reset old nodes, simulate again, check V_m changes for all.
3. Freeze half the nodes, simulate, check their V_m is constant.
4. Thaw nodes, simulate, check V_m changes again.

Author: Plesser
FirstVersion: 2014-11-05
"""

import numpy as np
import pytest
import nest

# SLI2PY: Needs guard against single-threaded

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
