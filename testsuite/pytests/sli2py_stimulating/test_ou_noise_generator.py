# -*- coding: utf-8 -*-
#
# test_noise_generator.py
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
Tests parameter setting and statistical correctness for one application.
"""

import nest
import numpy as np
import pytest


@pytest.fixture
def prepare_kernel():
    nest.ResetKernel()
    nest.resolution = 0.1


def test_ou_noise_generator_set_parameters(prepare_kernel):
    params = {"mean": 210.0, "std": 60.0, "dt": 0.1}

    oung1 = nest.Create("ou_noise_generator")
    oung1.set(params)

    nest.SetDefaults("ou_noise_generator", params)

    oung2 = nest.Create("ou_noise_generator")
    assert oung1.get(params) == oung2.get(params)


def test_ou_noise_generator_incorrect_noise_dt(prepare_kernel):
    with pytest.raises(nest.kernel.NESTError, match="StepMultipleRequired"):
        nest.Create("ou_noise_generator", {"dt": 0.25})


def test_ou_noise_generator_(prepare_kernel):
    # run for resolution dt=0.1 project to iaf_psc_alpha.
    # create 100 repetitions of 1000ms simulations
    # collect membrane potential at end
    ng = nest.Create("ou_noise_generator")
    neuron = nest.Create("iaf_psc_alpha")
    nest.Connect(ng, neuron)

    ng.set({"mean": 0.0, "std": 1.0, "dt": 0.1})

    # no spiking, all parameters 1, 0 leak potential
    neuron.set({"V_th": 1e10, "C_m": 1.0, "tau_m": 1.0, "E_L": 0.0})

    # Simulate for 100 times
    n_sims = 100
    v_m_arr = np.empty(n_sims)
    for i in range(n_sims):
        nest.Simulate(1000.0)
        v_m_arr[i] = neuron.get("V_m")

    # Mean and std of membrane potentials
    vm_mean = np.mean(v_m_arr)
    vm_std = np.std(v_m_arr)

    # Expected mean and std values
    expected_vm_mean = 0.0
    exp_dt = np.exp(-ng.get("dt") / neuron.get("tau_m"))
    expected_vm_std = np.sqrt((1 + exp_dt) / (1 - exp_dt)) * ng.get("std")
    expected_vm_std_std = expected_vm_std / np.sqrt(2)

    # require mean within 3 std dev, std dev within three std dev of std dev
    assert np.abs(vm_mean - expected_vm_mean) < 3 * expected_vm_std
    assert np.abs(vm_std - expected_vm_std) < 3 * expected_vm_std_std


def test_ou_noise_generator_variance(prepare_kernel):
    # run for resolution dt=0.1 project to iaf_psc_alpha.
    # create 100 repetitions of 1000ms simulations
    # collect membrane potential at end
    oung = nest.Create("ou_noise_generator")
    neuron = nest.Create("iaf_psc_alpha")
    # we need to connect to a neuron otherwise the generator does not generate
    nest.Connect(oung, neuron)

    oung.set({"mean": 0.0, "std": 1.0, "dt": 0.1})

    # no spiking, all parameters 1, 0 leak potential
    # neuron.set({"V_th": 1e10, "C_m": 1.0, "tau_m": 1.0, "E_L": 0.0})

    # Simulate for 100 times
    n_sims = 100
    v_m_arr = np.empty(n_sims)
    for i in range(n_sims):
        mm = nest.Create('multimeter', 1, {'record_from':['I']})
        nest.Connect(mm, oung)
        nest.Simulate(1000.0)

        ou_current = mm.get('events')['I']
        var_actual = np.var(ou_current)
        var_expected = oung.std**2

    # change this to check if the expected variance is close to the actual one

    '''
    # require mean within 3 std dev, std dev within three std dev of std dev
    assert np.abs(vm_mean - expected_vm_mean) < 3 * expected_vm_std
    assert np.abs(vm_std - expected_vm_std) < 3 * expected_vm_std_std
    '''
